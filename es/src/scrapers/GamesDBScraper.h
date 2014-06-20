#pragma once

#include "scrapers/Scraper.h"

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, 
	std::vector<ScraperSearchResult>& results);

class TheGamesDBRequest : public ScraperHttpRequest
{
public:
	TheGamesDBRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url) : ScraperHttpRequest(resultsWrite, url) {}
protected:
	void process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results) override;
};
