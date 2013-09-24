#include "Scraper.h"
#include "../components/AsyncReqComponent.h"

std::vector<MetaDataList> Scraper::getResults(ScraperSearchParams params)
{
	std::shared_ptr<HttpReq> req = makeHttpReq(params);
	while(req->status() == HttpReq::REQ_IN_PROGRESS);

	return parseReq(params, req);
}

void Scraper::getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc)
{
	std::shared_ptr<HttpReq> httpreq = makeHttpReq(params);
	AsyncReqComponent* req = new AsyncReqComponent(window, httpreq,
		[this, params, returnFunc] (std::shared_ptr<HttpReq> r)
	{
		returnFunc(parseReq(params, r));
	}, [returnFunc] ()
	{
		returnFunc(std::vector<MetaDataList>());
	});

	window->pushGui(req);
}
