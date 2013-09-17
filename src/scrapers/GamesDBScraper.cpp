#include "GamesDBScraper.h"
#include "../components/AsyncReqComponent.h"

std::vector<MetaDataList> GamesDBScraper::getResults(ScraperSearchParams params)
{
	std::shared_ptr<HttpReq> req = makeHttpReq();
	while(req->status() == HttpReq::REQ_IN_PROGRESS);

	return parseReq(params, req);
}

std::shared_ptr<HttpReq> GamesDBScraper::makeHttpReq()
{
	return std::make_shared<HttpReq>("cdn.garcya.us", "/wp-content/uploads/2010/04/TD250.jpg");
}

std::vector<MetaDataList> GamesDBScraper::parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq> req)
{
	std::vector<MetaDataList> mdl;

	return mdl;
}

void GamesDBScraper::getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc)
{
	std::shared_ptr<HttpReq> httpreq = makeHttpReq();
	AsyncReqComponent* req = new AsyncReqComponent(window, httpreq,
		[this, params, returnFunc] (std::shared_ptr<HttpReq> r)
	{
		returnFunc(parseReq(params, r));
	}, [] ()
	{
	});

	window->pushGui(req);
}

