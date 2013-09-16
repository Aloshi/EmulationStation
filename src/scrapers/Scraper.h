#pragma once

#include "../MetaData.h"
#include "../SystemData.h"
#include "../GameData.h"
#include <vector>

struct ScraperSearchParams
{
	SystemData* sys;
	GameData* game;

	std::string nameOverride;
	bool async;
};

class IScraper
{
public:
	//Get a list of potential results.
	virtual std::vector<MetaDataList> getResults(ScraperSearchParams params) = 0;
};

