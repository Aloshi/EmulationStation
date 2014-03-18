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

struct ScraperSearchResult
{
	ScraperSearchResult() : mdl(GAME_METADATA) {};

	MetaDataList mdl;
	std::string imageUrl;
	std::string thumbnailUrl;
};

enum ScraperSearchStatus
{
	SEARCH_IN_PROGRESS,
	SEARCH_ERROR,
	SEARCH_DONE
};

class ScraperSearchHandle
{
public:
	virtual void update() = 0;

	// Update and return the latest status.
	inline ScraperSearchStatus status() { update(); return mStatus; }

	// User-friendly string of our current status.  Will return error message if status() == SEARCH_ERROR.
	std::string getStatusString();

	inline const std::vector<ScraperSearchResult>& getResults() const { assert(mStatus != SEARCH_IN_PROGRESS); return mResults; }

protected:
	inline void setError(const std::string& error) { setStatus(SEARCH_ERROR); mError = error; }
	inline void setStatus(ScraperSearchStatus status) { mStatus = status; }
	inline void setResults(const std::vector<ScraperSearchResult>& results) { mResults = results; }

private:
	std::string mError;
	ScraperSearchStatus mStatus;
	std::vector<ScraperSearchResult> mResults;
};

class Scraper
{
public:
	//Get a list of potential results.
	virtual std::unique_ptr<ScraperSearchHandle> getResultsAsync(const ScraperSearchParams& params) = 0;

	virtual const char* getName() = 0;
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
