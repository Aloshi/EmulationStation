#pragma once

#include "scrapers/Scraper.h"

void thearchive_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results);

void thearchive_process_httpreq(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results);

class TheArchiveRequest : public ScraperHttpRequest
{
public:
	TheArchiveRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url) : ScraperHttpRequest(resultsWrite, url) {}
protected:
	void process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results) override;
};