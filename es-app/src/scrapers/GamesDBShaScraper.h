#pragma once

#include "scrapers/Scraper.h"

void thegamesdbsha_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results);

class TheGamesDBShaRequest : public ScraperHttpRequest
{
public:
	TheGamesDBShaRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url, std::string name);
	void set_name(std::string name);
protected:
	void process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results) override;
private:
	std::string noIntroName;
};
