#include "GamesDBScraper.h"
#include "../components/ScraperSearchComponent.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../pugiXML/pugixml.hpp"
#include "../MetaData.h"
#include "../Settings.h"
#include <boost/assign.hpp>

const char* GamesDBScraper::getName() { return "TheGamesDB"; }

using namespace PlatformIds;
const std::map<PlatformId, const char*> gamesdb_platformid_map = boost::assign::map_list_of
	(THREEDO, "3DO")
	(AMIGA, "Amiga")
	(ARCADE, "Arcade")
	(ATARI_2600, "Atari 2600")
	(ATARI_5200, "Atari 5200")
	(ATARI_7800, "Atari 7800")
	(ATARI_JAGUAR, "Atari Jaguar")
	(ATARI_JAGUAR_CD, "Atari Jaguar CD")
	(ATARI_XE, "Atari XE")
	(COLECOVISION, "Colecovision")
	(COMMODORE_64, "Commodore 64")
	(INTELLIVISION, "Intellivision")
	(MAC_OS, "Mac OS")
	(XBOX, "Microsoft Xbox")
	(XBOX_360, "Microsoft Xbox 360")
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
	(PLAYSTATION_VITA, "Sony Playstation Vita")
	(PLAYSTATION_PORTABLE, "Sony PSP")
	(SUPER_NINTENDO, "Super Nintendo (SNES)")
	(TURBOGRAFX_16, "TurboGrafx 16")
	(ZX_SPECTRUM, "Sinclair ZX Spectrum");


std::unique_ptr<ScraperSearchHandle> GamesDBScraper::getResultsAsync(const ScraperSearchParams& params)
{
	std::string path = "/api/GetGame.php?";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = params.game->getCleanName();
	
	path += "name=" + HttpReq::urlEncode(cleanName);

	if(params.system->getPlatformId() != PLATFORM_UNKNOWN)
	{
		path += "&platform=";
		path += HttpReq::urlEncode(gamesdb_platformid_map.at(params.system->getPlatformId()));
	}

	path = "thegamesdb.net" + path;

	return std::unique_ptr<ScraperSearchHandle>(new GamesDBHandle(params, path));
}

GamesDBHandle::GamesDBHandle(const ScraperSearchParams& params, const std::string& url) : 
	mReq(std::unique_ptr<HttpReq>(new HttpReq(url)))
{
	setStatus(ASYNC_IN_PROGRESS);
}

void GamesDBHandle::update()
{
	if(mStatus == ASYNC_DONE)
		return;

	if(mReq->status() == HttpReq::REQ_IN_PROGRESS)
		return;

	if(mReq->status() != HttpReq::REQ_SUCCESS)
	{
		std::stringstream ss;
		ss << "Network error - " << mReq->getErrorMsg();
		setError(ss.str());
		return;
	}

	// our HTTP request was successful
	// try to build our result list

	std::vector<ScraperSearchResult> results;

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(mReq->getContent().c_str());
	if(!parseResult)
	{
		setError("Error parsing XML");
		return;
	}

	pugi::xml_node data = doc.child("Data");

	std::string baseImageUrl = data.child("baseImgUrl").text().get();
	
	unsigned int resultNum = 0;
	pugi::xml_node game = data.child("Game");
	while(game && resultNum < MAX_SCRAPER_RESULTS)
	{
		ScraperSearchResult result;

		result.mdl.set("name", game.child("GameTitle").text().get());
		result.mdl.set("desc", game.child("Overview").text().get());

		boost::posix_time::ptime rd = string_to_ptime(game.child("ReleaseDate").text().get(), "%m/%d/%Y");
		result.mdl.setTime("releasedate", rd);

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

		resultNum++;
		game = game.next_sibling("Game");
	}

	setStatus(ASYNC_DONE);
	setResults(results);
	return;
}
