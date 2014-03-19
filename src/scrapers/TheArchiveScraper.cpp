#include "TheArchiveScraper.h"
#include "../guis/GuiGameScraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../pugiXML/pugixml.hpp"

const char* TheArchiveScraper::getName() { return "TheArchive"; }

std::unique_ptr<ScraperSearchHandle> TheArchiveScraper::getResultsAsync(const ScraperSearchParams& params)
{
	std::string path = "/2.0/Archive.search/xml/7TTRM4MNTIKR2NNAGASURHJOZJ3QXQC5/";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = getCleanFileName(params.game->getPath());
	
	path += HttpReq::urlEncode(cleanName);
	//platform TODO, should use some params.system get method

	path = "api.archive.vg" + path;

	return std::unique_ptr<ScraperSearchHandle>(new TheArchiveHandle(params, path));
}

TheArchiveHandle::TheArchiveHandle(const ScraperSearchParams& params, const std::string& url) : 
	mReq(std::unique_ptr<HttpReq>(new HttpReq(url)))
{
	setStatus(ASYNC_IN_PROGRESS);
}

void TheArchiveHandle::update()
{
	if(mStatus == ASYNC_DONE)
		return;

	if(mReq->status() == HttpReq::REQ_IN_PROGRESS)
		return;

	if(mReq->status() != HttpReq::REQ_SUCCESS)
	{
		std::stringstream ss;
		ss << "Network error: " << mReq->getErrorMsg();
		setError(ss.str());
		return;
	}

	// if we're here, our HTTP request finished successfully
	
	// so, let's try building our result list
	std::vector<ScraperSearchResult> results;

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(mReq->getContent().c_str());
	if(!parseResult)
	{
		setError("Error parsing XML");
		return;
	}

	pugi::xml_node data = doc.child("OpenSearchDescription").child("games");

	unsigned int resultNum = 0;
	pugi::xml_node game = data.child("game");
	while(game && resultNum < MAX_SCRAPER_RESULTS)
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

		resultNum++;
		game = game.next_sibling("game");
	}

	setStatus(ASYNC_DONE);
	setResults(results);
}
