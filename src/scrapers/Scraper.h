#pragma once

#include "../MetaData.h"
#include "../SystemData.h"
#include "../GameData.h"
#include "../HttpReq.h"
#include <vector>
#include <functional>

class Window;

struct ScraperSearchParams
{
	SystemData* system;
	GameData* game;

	std::string nameOverride;
};

class Scraper
{
public:
	//Get a list of potential results.
	virtual std::vector<MetaDataList> getResults(ScraperSearchParams params);
	virtual void getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc);

	virtual const char* getName() = 0;
private:
	virtual std::shared_ptr<HttpReq> makeHttpReq(ScraperSearchParams params) = 0;
	virtual std::vector<MetaDataList> parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq>) = 0;
};

