#include "TheArchiveScraper.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"

void thearchive_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	std::string path = "api.archive.vg/2.0/Archive.search/xml/7TTRM4MNTIKR2NNAGASURHJOZJ3QXQC5/";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = params.game->getCleanName();

	path += HttpReq::urlEncode(cleanName);
	//platform TODO, should use some params.system get method

	requests.push(std::unique_ptr<ScraperRequest>(new TheArchiveRequest(results, path)));
}

void TheArchiveRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	assert(req->status() == HttpReq::REQ_SUCCESS);

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(req->getContent().c_str());
	if(!parseResult)
	{
		std::stringstream ss;
		ss << "TheArchiveRequest - error parsing XML.\n\t" << parseResult.description();
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	pugi::xml_node data = doc.child("OpenSearchDescription").child("games");

	pugi::xml_node game = data.child("game");
	while(game && results.size() < MAX_SCRAPER_RESULTS)
	{
		ScraperSearchResult result;

		result.mdl.set("name", game.child("title").text().get());
		result.mdl.set("desc", game.child("description").text().get());

		//Archive.search does not return ratings

		result.mdl.set("developer", game.child("developer").text().get());

		std::string genre = game.child("genre").text().get();
		size_t search = genre.find_last_of(" &gt; ");
		genre = genre.substr(search == std::string::npos ? 0 : search, std::string::npos);
		result.mdl.set("genre", genre);

		pugi::xml_node image = game.child("box_front");
		pugi::xml_node thumbnail = game.child("box_front_small");

		if(image)
			result.imageUrl = image.text().get();
		if(thumbnail)
			result.thumbnailUrl = thumbnail.text().get();

		results.push_back(result);
		game = game.next_sibling("game");
	}
}
