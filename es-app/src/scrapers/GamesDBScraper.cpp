#include "EmulationStation.h"
#include "scrapers/GamesDBScraper.h"
#include "scrapers/json.hpp"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include "MetaData.h"
#include "Settings.h"
#include "Util.h"
#include <boost/assign.hpp>
#include <curl/curl.h>

using json = nlohmann::json;
using namespace PlatformIds;
const std::map<PlatformId, const char*> gamesdb_platformid_map = boost::assign::map_list_of
	(THREEDO, "3DO")
	(AMIGA, "Amiga")
	(AMSTRAD_CPC, "Amstrad CPC")
	// missing apple2
	(ARCADE, "Arcade")
	// missing atari 800
	(ATARI_2600, "Atari 2600")
	(ATARI_5200, "Atari 5200")
	(ATARI_7800, "Atari 7800")
	(ATARI_JAGUAR, "Atari Jaguar")
	(ATARI_JAGUAR_CD, "Atari Jaguar CD")
	(ATARI_LYNX, "Atari Lynx")
	// missing atari ST/STE/Falcon
	(ATARI_XE, "Atari XE")
	(COLECOVISION, "Colecovision")
	(COMMODORE_64, "Commodore 64")
	(INTELLIVISION, "Intellivision")
	(MAC_OS, "Mac OS")
	(XBOX, "Microsoft Xbox")
	(XBOX_360, "Microsoft Xbox 360")
	// missing MSX
	(NEOGEO, "NeoGeo")
	(NEOGEO_POCKET, "Neo Geo Pocket")
	(NEOGEO_POCKET_COLOR, "Neo Geo Pocket Color")
	(NINTENDO_3DS, "Nintendo 3DS")
	(NINTENDO_64, "Nintendo 64")
	(NINTENDO_DS, "Nintendo DS")
	(NINTENDO_ENTERTAINMENT_SYSTEM, "Nintendo Entertainment System (NES)")
	(GAME_BOY, "Nintendo Game Boy")
	(GAME_BOY_ADVANCE, "Nintendo Game Boy Advance")
	(GAME_BOY_COLOR, "Nintendo Game Boy Color")
	(NINTENDO_GAMECUBE, "Nintendo GameCube")
	(NINTENDO_WII, "Nintendo Wii")
	(NINTENDO_WII_U, "Nintendo Wii U")
	(PC, "PC")
	(SEGA_32X, "Sega 32X")
	(SEGA_CD, "Sega CD")
	(SEGA_DREAMCAST, "Sega Dreamcast")
	(SEGA_GAME_GEAR, "Sega Game Gear")
	(SEGA_GENESIS, "Sega Genesis")
	(SEGA_MASTER_SYSTEM, "Sega Master System")
	(SEGA_MEGA_DRIVE, "Sega Mega Drive")
	(SEGA_SATURN, "Sega Saturn")
	(PLAYSTATION, "Sony Playstation")
	(PLAYSTATION_2, "Sony Playstation 2")
	(PLAYSTATION_3, "Sony Playstation 3")
	(PLAYSTATION_4, "Sony Playstation 4")
	(PLAYSTATION_VITA, "Sony Playstation Vita")
	(PLAYSTATION_PORTABLE, "Sony PSP")
	(SUPER_NINTENDO, "Super Nintendo (SNES)")
	(TURBOGRAFX_16, "TurboGrafx 16")
	(WONDERSWAN, "WonderSwan")
	(WONDERSWAN_COLOR, "WonderSwan Color")
	(ZX_SPECTRUM, "Sinclair ZX Spectrum");

std::string TheGamesDBRequest::APIkey = "";
std::string TheGamesDBRequest::URL = "https://api.thegamesdb.net/"; // the default is the actual, true location.
std::unordered_map<unsigned int, std::string> TheGamesDBRequest::Genres = {};
std::unordered_map<unsigned int, std::string> TheGamesDBRequest::Developers = {};
std::unordered_map<unsigned int, std::string> TheGamesDBRequest::Publishers = {};

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, std::vector<ScraperSearchResult>& results) {
	std::string path = TheGamesDBRequest::URL + "Games/ByGameName?include=boxart&fields=players,genres,overview,developers,publishers&apikey=";

	std::string cleanName = params.nameOverride;
	if (cleanName.empty()) {
		cleanName = params.game->getCleanName();
	}

	path += TheGamesDBRequest::APIkey;
	path += "&name=" + HttpReq::urlEncode(cleanName);

	LOG(LogInfo) << "Path: " << path;

	// Platforms currently need to be referenced by an integral, unique identifier.
	if (params.system->getPlatformIds().empty()) {
		// no platform specified, we're done
		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, path)));
	} else {
		// go through the list, we need to split this into multiple requests
		// because TheGamesDB API either sucks or I don't know how to use it properly...
		std::string urlBase = path;
		auto& platforms = params.system->getPlatformIds();
		for (auto platformIt = platforms.begin(); platformIt != platforms.end(); platformIt++) {
			path = urlBase;
			auto mapIt = gamesdb_platformid_map.find(*platformIt);
			if (mapIt != gamesdb_platformid_map.end()) {
				path += "&filter=";
				path += HttpReq::urlEncode(mapIt->second);
			} else {
				LOG(LogWarning) << "TheGamesDB scraper warning - no support for platform " << getPlatformName(*platformIt);
			}
			LOG(LogInfo) << "request path: " << path;
	 		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, path)));
		}
	}
}

void TheGamesDBRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results) {
	assert(req->status() == HttpReq::REQ_SUCCESS);
	std::stringstream ss;

	json j;
	try {
		j = json::parse(req->getContent());
	} catch (const json::parse_error e) {
		ss << "GamesDBRequest - Error parsing JSON. \n\t" << e.what() << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	if (j["status"].empty() || j["status"] != "Success" || j["data"].empty() || j["data"]["count"].empty() || j["data"]["count"] < 1) {
		ss << "GamesDBRequest - Invalid response: " << j << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	json images = j["include"]["boxart"]["data"];
	std::string baseImageURL = j["include"]["boxart"]["base_url"]["large"];
	std::string baseThumbURL = j["include"]["boxart"]["base_url"]["thumb"];
	// bool getRatings = Settings::getInstance()->getBool("ScrapeRatings");

	for (auto g : j["data"]["games"]) {
		ScraperSearchResult result;

		if (g["game_title"].is_string()) {
			result.mdl.set("name", g["game_title"]);
		}

		if (g["overview"].is_string()) {
			result.mdl.set("desc", g["overview"]);
		}

		if (g["release_date"].is_string()) {
			result.mdl.setTime("releasedate", string_to_ptime(g["release_date"], "%Y-%m-%d"));
		}

		if (g["players"].is_number()) {
			result.mdl.set("players", std::to_string((unsigned int)g["players"]));
		}

		if (g["genres"].is_array() && g["genres"][0].is_number()) {
			std::string genre = TheGamesDBRequest::getGenreByID((unsigned int)(g["genres"][0]));
			result.mdl.set("genre", genre);
		}

		if (g["developers"].is_array() && !g["developers"].empty() && g["developers"][0].is_number()) {
			std::string dev = TheGamesDBRequest::getDeveloperByID((unsigned int)(g["developers"][0]));
			result.mdl.set("developer", dev);
		}

		if (g["publishers"].is_array() && g["publishers"][0].is_number()) {
			std::string publisher = TheGamesDBRequest::getPublisherByID((unsigned int)(g["developers"][0]));
			LOG(LogDebug) << "Publisher: " << publisher;
			result.mdl.set("publisher", publisher);
		}

		// ratings mean something different these days
		// if (!g["rating"].empty()) {
		// 	ss << g["rating"];
		// 	result.mdl.set("rating", g["rating"]);
		// 	ss.clear();
		// }

		std::string id = std::to_string((unsigned long long)g["id"]);

		if (images[id].is_array() && !images[id].empty()) {
			for (auto i : images[id]) {
				if (i["side"].is_string() && i["side"] == "front" && i["type"].is_string() && i["type"] == "boxart" && i["filename"].is_string()) {
					result.imageUrl = baseImageURL + i["filename"].get<std::string>();
					result.thumbnailUrl = baseThumbURL + i["filename"].get<std::string>();
					break;
				}
			}
		}

		results.push_back(result);
	}
}

size_t genericWrite(void* buffer, size_t size, size_t nmemb, std::string* output) {
	output->append((char*)buffer, size*nmemb);
	return nmemb*size;
}

CURLcode simpleSetup(CURL* handle, const char* URL) {
	CURLcode err = curl_easy_setopt(handle, CURLOPT_URL, URL);
	if (err != CURLE_OK) {
		LOG(LogError) << "Error setting curl URL: " << curl_easy_strerror(err);
		return err;
	}

	err = curl_easy_setopt(handle, CURLOPT_USERAGENT, USER_AGENT_STRING);
	if (err != CURLE_OK) {
		LOG(LogError) << "Error setting curl User-Agent: " << curl_easy_strerror(err);
		return err;
	}

	err = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, genericWrite);
	if (err != CURLE_OK) {
		LOG(LogError) << "Error assigning curl write callback: " << curl_easy_strerror(err);
		return err;
	}

	return CURLE_OK;
}

CURLcode simpleSetup(CURL* handle, std::string URL) {
	return simpleSetup(handle, URL.c_str());
}

std::string TheGamesDBRequest::getGenreByID(unsigned int id) {
	if (TheGamesDBRequest::Genres.empty()) {
		auto curl = curl_easy_init();
		if (!curl) {
			LOG(LogError) << "Failed to initialize genres curl.";
			return "";
		}

		std::string url = TheGamesDBRequest::URL + "Genres?apikey=";
		url += TheGamesDBRequest::APIkey;
		auto err = simpleSetup(curl, url);
		if (err != CURLE_OK) {
			return "";
		}

		std::string responseBody;
		err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
		if (err != CURLE_OK) {
			LOG(LogError) << "Error capturing curl response body: " << curl_easy_strerror(err);
			return "";
		}

		err = curl_easy_perform(curl);
		if (err != CURLE_OK) {
			LOG(LogError) << "Error performing curl: " << curl_easy_strerror(err);
			return "";
		}

		curl_easy_cleanup(curl);
		curl = nullptr;

		json r;
		try {
			r = json::parse(responseBody);
		} catch (const json::parse_error e) {
			std::stringstream ss;
			ss << "Genre Request - Error parsing JSON.\n\t" << e.what() << "";
			std::string err = ss.str();
			LOG(LogError) << err;
			return "";
		}

		if (r.is_array()) {
			r = r[0]; //TheGamesDB does this weird thing where they have a top-level array containing the actual data array
		}

		if (r["data"].is_object() && r["data"]["genres"].is_object()) {
			for (auto g = r["data"]["genres"].begin(); g != r["data"]["genres"].end(); ++g) {
				TheGamesDBRequest::Genres[(unsigned int)(g.value()["id"])] = g.value()["name"];
			}
		} else {
			LOG(LogError) << "Malformed API response when fetching genres: " << r;
			return "";
		}

	}

	return TheGamesDBRequest::Genres[id];
}

std::string TheGamesDBRequest::getDeveloperByID(unsigned int id) {
	if (TheGamesDBRequest::Developers.empty()) {
		auto curl = curl_easy_init();
		if (!curl) {
			LOG(LogError) << "Failed to initialize developers curl.";
			return "";
		}

		std::string url = TheGamesDBRequest::URL + "Developers?apikey=";
		url += TheGamesDBRequest::APIkey;
		auto err = simpleSetup(curl, url);
		if (err != CURLE_OK) {
			return "";
		}

		std::string responseBody;
		err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
		if (err != CURLE_OK) {
			LOG(LogError) << "Error capturing curl response body: " << curl_easy_strerror(err);
			return "";
		}

		err = curl_easy_perform(curl);
		if (err != CURLE_OK) {
			LOG(LogError) << "Error performing curl: " << curl_easy_strerror(err);
			return "";
		}

		curl_easy_cleanup(curl);
		curl = nullptr;

		json r;
		try {
			r = json::parse(responseBody);
		} catch (const json::parse_error e) {
			std::stringstream ss;
			ss << "Developer Request - Error parsing JSON.\n\t" << e.what() << "";
			std::string err = ss.str();
			LOG(LogError) << err;
			return "";
		}


		if (r.is_array()) {
			r = r[0]; //TheGamesDB does this weird thing where they have a top-level array containing the actual data array
		}

		if (r["data"].is_object() && r["data"]["developers"].is_object()) {
			for (auto d = r["data"]["developers"].begin(); d != r["data"]["developers"].end(); ++d) {
				TheGamesDBRequest::Developers[(unsigned int)(d.value()["id"])] = d.value()["name"];
			}
		} else {
			LOG(LogError) << "Malformed API response when fetching developers: " << r;
			return "";
		}

	}

	return TheGamesDBRequest::Developers[id];
}

std::string TheGamesDBRequest::getPublisherByID(unsigned int id) {
	if (TheGamesDBRequest::Publishers.empty()) {
		auto curl = curl_easy_init();
		if (!curl) {
			LOG(LogError) << "Failed to initialize developers curl.";
			return "";
		}

		std::string url = TheGamesDBRequest::URL + "Publishers?apikey=";
		url += TheGamesDBRequest::APIkey;
		auto err = simpleSetup(curl, url);
		if (err != CURLE_OK) {
			return "";
		}

		std::string responseBody;
		err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
		if (err != CURLE_OK) {
			LOG(LogError) << "Error capturing curl response body: " << curl_easy_strerror(err);
			return "";
		}

		err = curl_easy_perform(curl);
		if (err != CURLE_OK) {
			LOG(LogError) << "Error performing curl: " << curl_easy_strerror(err);
			return "";
		}

		curl_easy_cleanup(curl);
		curl = nullptr;

		json r;
		try {
			r = json::parse(responseBody);
		} catch (const json::parse_error e) {
			std::stringstream ss;
			ss << "Publisher Request - Error parsing JSON.\n\t" << e.what() << "";
			std::string err = ss.str();
			LOG(LogError) << err;
			return "";
		}


		if (r.is_array()) {
			r = r[0]; //TheGamesDB does this weird thing where they have a top-level array containing the actual data array
		}

		if (r["data"].is_object() && r["data"]["publishers"].is_object()) {
			for (auto p = r["data"]["publishers"].begin(); p != r["data"]["publishers"].end(); ++p) {
				TheGamesDBRequest::Publishers[(unsigned int)(p.value()["id"])] = p.value()["name"];
			}
		} else {
			LOG(LogError) << "Malformed API response when fetching publishers: " << r;
			return "";
		}

	}

	return TheGamesDBRequest::Publishers[id];
}
