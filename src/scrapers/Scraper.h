#pragma once

#include "../MetaData.h"
#include "../SystemData.h"
#include "../GameData.h"
#include <vector>
#include <functional>

class Window;

struct ScraperSearchParams
{
	SystemData* system;
	GameData* game;

	std::string nameOverride;
};

class IScraper
{
public:
	//Get a list of potential results.
	virtual std::vector<MetaDataList> getResults(ScraperSearchParams params) = 0;
	virtual void getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc) = 0;
};

