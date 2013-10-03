#pragma once

#include "Scraper.h"
#include "../HttpReq.h"

class GamesDBScraper : public Scraper
{
public:
	const char* getName();
private:
	std::shared_ptr<HttpReq> makeHttpReq(ScraperSearchParams params) override;
	std::vector<MetaDataList> parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq>) override;
};
