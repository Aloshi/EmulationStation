#pragma once

#include "Scraper.h"

class GamesDBScraper : public IScraper
{
public:
	std::vector<MetaDataList> getResults(ScraperSearchParams params) override;
};

