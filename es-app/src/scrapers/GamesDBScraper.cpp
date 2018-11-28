#include "scrapers/GamesDBScraper.h"

#include "utils/TimeUtil.h"
#include "FileData.h"
#include "Log.h"
#include "PlatformId.h"
#include "Settings.h"
#include "SystemData.h"
#include <pugixml/src/pugixml.hpp>

using namespace PlatformIds;
const std::map<PlatformId, const char*> gamesdb_platformid_map {
	{ THREEDO, "3DO" },
	{ AMIGA, "Amiga" },
	{ AMSTRAD_CPC, "Amstrad CPC" },
	// missing apple2
	{ ARCADE, "Arcade" },
	// missing atari 800
	{ ATARI_2600, "Atari 2600" },
	{ ATARI_5200, "Atari 5200" },
	{ ATARI_7800, "Atari 7800" },
	{ ATARI_JAGUAR, "Atari Jaguar" },
	{ ATARI_JAGUAR_CD, "Atari Jaguar CD" },
	{ ATARI_LYNX, "Atari Lynx" },
	// missing atari ST/STE/Falcon
	{ ATARI_XE, "Atari XE" },
	{ COLECOVISION, "Colecovision" },
	{ COMMODORE_64, "Commodore 64" },
	{ INTELLIVISION, "Intellivision" },
	{ MAC_OS, "Mac OS" },
	{ XBOX, "Microsoft Xbox" },
	{ XBOX_360, "Microsoft Xbox 360" },
	{ MSX, "MSX" },
	{ NEOGEO, "Neo Geo" },
	{ NEOGEO_POCKET, "Neo Geo Pocket" },
	{ NEOGEO_POCKET_COLOR, "Neo Geo Pocket Color" },
	{ NINTENDO_3DS, "Nintendo 3DS" },
	{ NINTENDO_64, "Nintendo 64" },
	{ NINTENDO_DS, "Nintendo DS" },
	{ FAMICOM_DISK_SYSTEM, "Famicom Disk System" },
	{ NINTENDO_ENTERTAINMENT_SYSTEM, "Nintendo Entertainment System (NES)" },
	{ GAME_BOY, "Nintendo Game Boy" },
	{ GAME_BOY_ADVANCE, "Nintendo Game Boy Advance" },
	{ GAME_BOY_COLOR, "Nintendo Game Boy Color" },
	{ NINTENDO_GAMECUBE, "Nintendo GameCube" },
	{ NINTENDO_WII, "Nintendo Wii" },
	{ NINTENDO_WII_U, "Nintendo Wii U" },
	{ NINTENDO_VIRTUAL_BOY, "Nintendo Virtual Boy" },
	{ NINTENDO_GAME_AND_WATCH, "Game &amp; Watch" },
	{ PC, "PC" },
	{ SEGA_32X, "Sega 32X" },
	{ SEGA_CD, "Sega CD" },
	{ SEGA_DREAMCAST, "Sega Dreamcast" },
	{ SEGA_GAME_GEAR, "Sega Game Gear" },
	{ SEGA_GENESIS, "Sega Genesis" },
	{ SEGA_MASTER_SYSTEM, "Sega Master System" },
	{ SEGA_MEGA_DRIVE, "Sega Mega Drive" },
	{ SEGA_SATURN, "Sega Saturn" },
	{ SEGA_SG1000, "SEGA SG-1000" },	
	{ PLAYSTATION, "Sony Playstation" },
	{ PLAYSTATION_2, "Sony Playstation 2" },
	{ PLAYSTATION_3, "Sony Playstation 3" },
	{ PLAYSTATION_4, "Sony Playstation 4" },
	{ PLAYSTATION_VITA, "Sony Playstation Vita" },
	{ PLAYSTATION_PORTABLE, "Sony Playstation Portable" },
	{ SUPER_NINTENDO, "Super Nintendo (SNES)" },
	{ TURBOGRAFX_16, "TurboGrafx 16" }, // HuCards only
	{ TURBOGRAFX_CD, "TurboGrafx CD" }, // CD-ROMs only
	{ WONDERSWAN, "WonderSwan" },
	{ WONDERSWAN_COLOR, "WonderSwan Color" },
	{ ZX_SPECTRUM, "Sinclair ZX Spectrum" },
	{ VIDEOPAC_ODYSSEY2, "Magnavox Odyssey 2" },
	{ VECTREX, "Vectrex" },
	{ TRS80_COLOR_COMPUTER, "TRS-80 Color Computer" },
	{ TANDY, "TRS-80 Color Computer" }
};

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, 
	std::vector<ScraperSearchResult>& results)
{
	std::string path;
	bool usingGameID = false;

	std::string cleanName = params.nameOverride;
	if (!cleanName.empty() && cleanName.substr(0,3) == "id:")
	{
		std::string gameID = cleanName.substr(3);
		path = "legacy.thegamesdb.net/api/GetGame.php?id=" + HttpReq::urlEncode(gameID);
		usingGameID = true;
	}else{
		if (cleanName.empty())
			cleanName = params.game->getCleanName();
		path += "legacy.thegamesdb.net/api/GetGamesList.php?name=" + HttpReq::urlEncode(cleanName);
	}

	if(usingGameID)
	{
		// if we have the ID already, we don't need the GetGameList request
		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, path)));
	}else if(params.system->getPlatformIds().empty()){
		// no platform specified, we're done
		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(requests, results, path)));
	}else{
		// go through the list, we need to split this into multiple requests 
		// because TheGamesDB API either sucks or I don't know how to use it properly...
		std::string urlBase = path;
		auto& platforms = params.system->getPlatformIds();
		for(auto platformIt = platforms.cbegin(); platformIt != platforms.cend(); platformIt++)
		{
			path = urlBase;
			auto mapIt = gamesdb_platformid_map.find(*platformIt);
			if(mapIt != gamesdb_platformid_map.cend())
			{
				path += "&platform=";
				path += HttpReq::urlEncode(mapIt->second);
			}else{
				LOG(LogWarning) << "TheGamesDB scraper warning - no support for platform " << getPlatformName(*platformIt);
			}

			requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(requests, results, path)));
		}
	}
}

void TheGamesDBRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	assert(req->status() == HttpReq::REQ_SUCCESS);

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(req->getContent().c_str());
	if(!parseResult)
	{
		std::stringstream ss;
		ss << "TheGamesDBRequest - Error parsing XML. \n\t" << parseResult.description() << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	if (isGameRequest())
		processGame(doc, results);
	else
		processList(doc, results);
}

void TheGamesDBRequest::processGame(const pugi::xml_document& xmldoc, std::vector<ScraperSearchResult>& results)
{
	pugi::xml_node data = xmldoc.child("Data");

	std::string baseImageUrl = data.child("baseImgUrl").text().get();

	pugi::xml_node game = data.child("Game");
	if(game)
	{
		ScraperSearchResult result;

		result.mdl.set("name", game.child("GameTitle").text().get());
		result.mdl.set("desc", game.child("Overview").text().get());
		result.mdl.set("releasedate", Utils::Time::DateTime(Utils::Time::stringToTime(game.child("ReleaseDate").text().get(), "%m/%d/%Y")));
		result.mdl.set("developer", game.child("Developer").text().get());
		result.mdl.set("publisher", game.child("Publisher").text().get());
		result.mdl.set("genre", game.child("Genres").first_child().text().get());
		result.mdl.set("players", game.child("Players").text().get());

		if(Settings::getInstance()->getBool("ScrapeRatings") && game.child("Rating"))
		{
			float ratingVal = (game.child("Rating").text().as_int() / 10.0f);
			std::stringstream ss;
			ss << ratingVal;
			result.mdl.set("rating", ss.str());
		}

		pugi::xml_node images = game.child("Images");

		if(images)
		{
			pugi::xml_node art = images.find_child_by_attribute("boxart", "side", "front");

			if(art)
			{
				result.thumbnailUrl = baseImageUrl + art.attribute("thumb").as_string();
				result.imageUrl = baseImageUrl + art.text().get();
			}
		}

		results.push_back(result);
	}
}

void TheGamesDBRequest::processList(const pugi::xml_document& xmldoc, std::vector<ScraperSearchResult>& results)
{
	assert(mRequestQueue != nullptr);

	pugi::xml_node data = xmldoc.child("Data");
	pugi::xml_node game = data.child("Game");

	// limit the number of results per platform, not in total.
	// otherwise if the first platform returns >= 7 games
	// but the second platform contains the relevant game,
	// the relevant result would not be shown.
	for(int i = 0; game && i < MAX_SCRAPER_RESULTS; i++)
	{
		std::string id = game.child("id").text().get();
		std::string path = "legacy.thegamesdb.net/api/GetGame.php?id=" + id;

		mRequestQueue->push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, path)));

		game = game.next_sibling("Game");
	}
}
