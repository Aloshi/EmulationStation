#pragma once

#include <unordered_map>
#include "scrapers/Scraper.h"

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results);

class TheGamesDBRequest : public ScraperHttpRequest
{
public:
	static std::string APIkey;
	static std::string Path;
	static std::unordered_map<unsigned int, std::string> Genres;
	static std::unordered_map<unsigned int, std::string> Developers;
	static std::unordered_map<unsigned int, std::string> Publishers;
	TheGamesDBRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url) : ScraperHttpRequest(resultsWrite, url) {}
	static std::string getGenreByID(unsigned int);
	static std::string getDeveloperByID(unsigned int);
	static std::string getPublisherByID(unsigned int);
protected:
	void process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results) override;
};
