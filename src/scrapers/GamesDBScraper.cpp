#include "GamesDBScraper.h"
#include "../guis/GuiGameScraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../pugiXML/pugixml.hpp"
#include "../MetaData.h"
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
	(TURBOGRAFX_16, "TurboGrafx 16");


std::shared_ptr<HttpReq> GamesDBScraper::makeHttpReq(ScraperSearchParams params)
{
	std::string path = "/api/GetGame.php?";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = getCleanFileName(params.game->getPath());
	
	path += "name=" + HttpReq::urlEncode(cleanName);

	if(params.system->getPlatformId() != PLATFORM_UNKNOWN)
	{
		path += "&platform=";
		path += HttpReq::urlEncode(gamesdb_platformid_map.at(params.system->getPlatformId()));
	}

	return std::make_shared<HttpReq>("thegamesdb.net" + path);
}

std::vector<MetaDataList> GamesDBScraper::parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq> req)
{
	std::vector<MetaDataList> mdl;

	if(req->status() != HttpReq::REQ_SUCCESS)
	{
		LOG(LogError) << "HttpReq error";
		return mdl;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(req->getContent().c_str());
	if(!parseResult)
	{
		LOG(LogError) << "Error parsing XML";
		return mdl;
	}

	pugi::xml_node data = doc.child("Data");

	std::string baseImageUrl = data.child("baseImgUrl").text().get();
	
	unsigned int resultNum = 0;
	pugi::xml_node game = data.child("Game");
	while(game && resultNum < MAX_SCRAPER_RESULTS)
	{
		mdl.push_back(MetaDataList(GAME_METADATA));
		mdl.back().set("name", game.child("GameTitle").text().get());
		mdl.back().set("desc", game.child("Overview").text().get());

		boost::posix_time::ptime rd = string_to_ptime(game.child("ReleaseDate").text().get(), "%m/%d/%Y");
		mdl.back().setTime("releasedate", rd);

		mdl.back().set("developer", game.child("Developer").text().get());
		mdl.back().set("publisher", game.child("Publisher").text().get());
		mdl.back().set("genre", game.child("Genres").first_child().text().get());
		mdl.back().set("players", game.child("Players").text().get());

		if(Settings::getInstance()->getBool("ScrapeRatings") && game.child("Rating"))
		{
			float ratingVal = (game.child("Rating").text().as_int() / 10.0f);
			std::stringstream ss;
			ss << ratingVal;
			mdl.back().set("rating", ss.str());
		}

		pugi::xml_node images = game.child("Images");

		if(images)
		{
			pugi::xml_node art = images.find_child_by_attribute("boxart", "side", "front");

			if(art)
			{
				mdl.back().set("thumbnail", baseImageUrl + art.attribute("thumb").as_string());
				mdl.back().set("image", baseImageUrl + art.text().get());
			}
		}

		resultNum++;
		game = game.next_sibling("Game");
	}

	return mdl;
}
