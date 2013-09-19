#include "GamesDBScraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"

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

	MetaDataList md(params.system->getGameMDD());
	md.set("name", "JUNK RESULT #1");
	md.set("desc", "Black triangles");
	mdl.push_back(md);

	MetaDataList md2(params.system->getGameMDD());
	md2.set("name", "JUNK RESULT #2");
	md2.set("desc", "Test results are very exciting. Sort of. A little. If you squint. A lot.");
	mdl.push_back(md2);

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
