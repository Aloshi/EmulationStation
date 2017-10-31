#pragma once
#ifndef ES_APP_SCRAPERS_GAMES_DB_SCRAPER_H
#define ES_APP_SCRAPERS_GAMES_DB_SCRAPER_H

#include "scrapers/Scraper.h"

namespace pugi { class xml_document; }

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, 
	std::vector<ScraperSearchResult>& results);

class TheGamesDBRequest : public ScraperHttpRequest
{
public:
	// ctor for a GetGameList request
	TheGamesDBRequest(std::queue< std::unique_ptr<ScraperRequest> >& requestsWrite, std::vector<ScraperSearchResult>& resultsWrite, const std::string& url) : ScraperHttpRequest(resultsWrite, url), mRequestQueue(&requestsWrite) {}
	// ctor for a GetGame request
	TheGamesDBRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url) : ScraperHttpRequest(resultsWrite, url), mRequestQueue(nullptr) {}

protected:
	void process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results) override;
	void processList(const pugi::xml_document& xmldoc, std::vector<ScraperSearchResult>& results);
	void processGame(const pugi::xml_document& xmldoc, std::vector<ScraperSearchResult>& results);
	bool isGameRequest() { return !mRequestQueue; }

	std::queue< std::unique_ptr<ScraperRequest> >* mRequestQueue;
};

#endif // ES_APP_SCRAPERS_GAMES_DB_SCRAPER_H
