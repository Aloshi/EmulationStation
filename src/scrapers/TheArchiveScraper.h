#pragma once

#include "Scraper.h"
#include "../HttpReq.h"

class TheArchiveScraper : public IScraper
{
public:
	std::vector<MetaDataList> getResults(ScraperSearchParams params) override;
	void getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc) override;

private:
	std::shared_ptr<HttpReq> makeHttpReq(ScraperSearchParams params);
	std::vector<MetaDataList> parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq>);
};

