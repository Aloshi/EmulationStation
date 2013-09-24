#include "TheArchiveScraper.h"
#include "../components/GuiGameScraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../pugiXML/pugixml.hpp"

std::vector<MetaDataList> TheArchiveScraper::getResults(ScraperSearchParams params)
{
	std::shared_ptr<HttpReq> req = makeHttpReq(params);
	while(req->status() == HttpReq::REQ_IN_PROGRESS);

	return parseReq(params, req);
}

std::shared_ptr<HttpReq> TheArchiveScraper::makeHttpReq(ScraperSearchParams params)
{
	std::string path = "/2.0/Archive.search/xml/7TTRM4MNTIKR2NNAGASURHJOZJ3QXQC5/";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = params.game->getCleanName();
	
	path += HttpReq::urlEncode(cleanName);
	//platform TODO, should use some params.system get method

	return std::make_shared<HttpReq>("api.archive.vg", path);
}

std::vector<MetaDataList> TheArchiveScraper::parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq> req)
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

	pugi::xml_node data = doc.child("OpenSearchDescription").child("games");

	unsigned int resultNum = 0;
	pugi::xml_node game = data.child("game");
	while(game && resultNum < MAX_SCRAPER_RESULTS)
	{
		mdl.push_back(MetaDataList(params.system->getGameMDD()));
		mdl.back().set("name", game.child("title").text().get());
		mdl.back().set("desc", game.child("description").text().get());
		
		pugi::xml_node image = game.child("box_front");
		pugi::xml_node thumbnail = game.child("box_front_small");

		if (image)
			mdl.back().set("image",image.text().get());
		if (thumbnail)
			mdl.back().set("thumbnail", thumbnail.text().get());

		resultNum++;
		game = game.next_sibling("game");
	}

	return mdl;
}

void TheArchiveScraper::getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc)
{
	std::shared_ptr<HttpReq> httpreq = makeHttpReq(params);
	AsyncReqComponent* req = new AsyncReqComponent(window, httpreq,
		[this, params, returnFunc] (std::shared_ptr<HttpReq> r)
	{
		returnFunc(parseReq(params, r));
	}, [] ()
	{
	});

	window->pushGui(req);
}
