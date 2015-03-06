#include "scrapers/GamesDBShaScraper.h"
#include "HttpReq.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include "MetaData.h"
#include "Settings.h"
#include "Util.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <map>
#include <vector>
#include "scrapers/ROMHasher.h"

namespace GamesDBShaScraper {

// TODO: Convert to on disk embeddable DB like sqlite/leveldb.
std::map<std::string, std::string> hash_to_id;
std::map<std::string, std::string> hash_to_name;

boost::filesystem::path get_hash_path()
{
	std::string home = getHomePath();
	boost::filesystem::path hash_path = home + "/.emulationstation/rom_hashes";
	boost::filesystem::create_directories(hash_path);
	hash_path = hash_path / "hash.tsv";
	return hash_path;
}

// Gets or fetches the hash file returning the path.
boost::filesystem::path get_or_fetch_hash_file()
{
	boost::filesystem::path hash_path = get_hash_path();
	if (boost::filesystem::exists(hash_path))
	{
		std::time_t now = time(NULL);
		std::time_t mtime = boost::filesystem::last_write_time(hash_path);
		double seconds = std::difftime(now, mtime);
		if (seconds <= 14400)  // File <4h old so reuse it..
		{
			return hash_path;
		}
	}
	HttpReq myRequest("storage.googleapis.com/stevenselph.appspot.com/hash.tsv");
	while(myRequest.status() == HttpReq::REQ_IN_PROGRESS);
	if(myRequest.status() != HttpReq::REQ_SUCCESS)
	{
		//an error occured
		LOG(LogError) << "HTTP request error - " << myRequest.getErrorMsg();
		return boost::filesystem::path ("");
	}
	std::ofstream ofile (hash_path.c_str());
	ofile << myRequest.getContent();
	ofile.close();
	return hash_path;
}

// Initialize the in memory mappings of hash to id and name.
void init_hash_maps ()
{
	boost::filesystem::path hash_path = get_or_fetch_hash_file();
	if (hash_path.empty())
	{
		return;
	}
	std::ifstream file (hash_path.c_str());
	if (!file)
	{
		return;
	}
	std::string line;
	while(std::getline(file, line))
	{
		std::istringstream s(line);
		int c = 0;
		std::string field, key, id, name;
		while (std::getline(s, field, '\t'))
		{
			switch (c)
			{
				case 0:
					key = field;
				case 1:
					id = field;
				case 3:
					name = field;
			}
			c++;
		}
		hash_to_id[key] = id;
		hash_to_name[key] = name;
	}
}

// Get the hash to ID map.
std::map<std::string, std::string> get_hash_to_id ()
{
	if (hash_to_id.empty())
	{
		init_hash_maps();
	}
	return hash_to_id;
}

// Get the hash to name map.
std::map<std::string, std::string> get_hash_to_name ()
{
	if (hash_to_name.empty())
	{
		init_hash_maps();
	}
	return hash_to_name;
}

} // end namespace

using namespace GamesDBShaScraper;

void thegamesdbsha_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	std::map<std::string, std::string> hm = get_hash_to_id();
	std::map<std::string, std::string> nm = get_hash_to_name();
	std::string path = "thegamesdb.net/api/GetGame.php?";

	std::string h = ROMHasher::sha1_rom(params.game.getPath());
	std::string name = "";
	if (nm.count(h) > 0)
	{
		name = nm[h];
	}

	if (hm.count(h) > 0 && hm[h] != "")
	{
		path += "id=" + hm[h];
		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBShaRequest(results, path, name)));
	}
}

TheGamesDBShaRequest::TheGamesDBShaRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url, std::string name)
	: ScraperHttpRequest(resultsWrite, url)
{
	noIntroName = name;
}

void TheGamesDBShaRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	assert(req->status() == HttpReq::REQ_SUCCESS);

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(req->getContent().c_str());
	if(!parseResult)
	{
		std::stringstream ss;
		ss << "GamesDBRequest - Error parsing XML. \n\t" << parseResult.description() << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	pugi::xml_node data = doc.child("Data");

	std::string baseImageUrl = data.child("baseImgUrl").text().get();

	pugi::xml_node game = data.child("Game");
	while(game && results.size() < 1)
	{
		ScraperSearchResult result;

		if (noIntroName.empty())
		{
			result.metadata.set("name", game.child("GameTitle").text().get());
		}
		else
		{
			result.metadata.set("name", noIntroName);
		}
		result.metadata.set("desc", game.child("Overview").text().get());

		boost::posix_time::ptime rd = string_to_ptime(game.child("ReleaseDate").text().get(), "%m/%d/%Y");
		result.metadata.set("releasedate", rd);

		result.metadata.set("developer", game.child("Developer").text().get());
		result.metadata.set("publisher", game.child("Publisher").text().get());
		result.metadata.set("genre", game.child("Genres").first_child().text().get());
		result.metadata.set("players", game.child("Players").text().get());

		if(Settings::getInstance()->getBool("ScrapeRatings") && game.child("Rating"))
		{
			float ratingVal = (game.child("Rating").text().as_int() / 10.0f);
			std::stringstream ss;
			ss << ratingVal;
			result.metadata.set("rating", ss.str());
		}

		pugi::xml_node images = game.child("Images");

		if(images)
		{
			pugi::xml_node art = images.find_child_by_attribute("boxart", "side", "front");

			if(art)
			{
				result.thumbnailUrl = baseImageUrl + art.attribute("thumb").as_string();
				result.imageUrl = baseImageUrl + art.attribute("thumb").as_string();
			}
		}

		results.push_back(result);
		game = game.next_sibling("Game");
	}
}
