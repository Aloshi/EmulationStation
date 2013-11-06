#pragma once

#include "../MetaData.h"
#include "../SystemData.h"
#include "../HttpReq.h"
#include <vector>
#include <functional>

class Window;

struct ScraperSearchParams
{
	SystemData* system;
	FileData* game;

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

std::shared_ptr<Scraper> createScraperByName(const std::string& name);

//About the same as "~/.emulationstation/downloaded_images/[system_name]/[game_name].[url's extension]".
//Will create the "downloaded_images" and "subdirectory" directories if they do not exist.
std::string getSaveAsPath(const ScraperSearchParams& params, const std::string& suffix, const std::string& url);

//Returns the path to the downloaded file (saveAs) on completion.
//Returns empty string if an error occured.
//Will resize according to Settings::getInt("ScraperResizeWidth") and Settings::getInt("ScraperResizeHeight").
std::string downloadImage(const std::string& url, const std::string& saveAs);

//Returns (via returnFunc) the path to the downloaded file (saveAs) on completion.
//Returns empty string if an error occured.
//Will resize according to Settings::getInt("ScraperResizeWidth") and Settings::getInt("ScraperResizeHeight").
//Same as downloadImage, just async.
void downloadImageAsync(Window* window, const std::string& url, const std::string& saveAs, std::function<void(std::string)> returnFunc);

void resolveMetaDataAssetsAsync(Window* window, const ScraperSearchParams& params, MetaDataList mdl, std::function<void(MetaDataList)> returnFunc);

//You can pass 0 for maxWidth or maxHeight to automatically keep the aspect ratio.
//Will overwrite the image at [path] with the new resized one.
void resizeImage(const std::string& path, int maxWidth, int maxHeight);
