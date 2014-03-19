#pragma once

#include "../MetaData.h"
#include "../SystemData.h"
#include "../HttpReq.h"
#include "../AsyncHandle.h"
#include <vector>
#include <functional>

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

class ScraperSearchHandle : public AsyncHandle
{
public:
	virtual void update() = 0;
	inline const std::vector<ScraperSearchResult>& getResults() const { assert(mStatus != ASYNC_IN_PROGRESS); return mResults; }

protected:
	inline void setResults(const std::vector<ScraperSearchResult>& results) { mResults = results; }

private:
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


// Meta data asset downloading stuff.
class MDResolveHandle : public AsyncHandle
{
public:
	MDResolveHandle(const ScraperSearchResult& result, const ScraperSearchParams& search);

	void update() override;
	inline const ScraperSearchResult& getResult() const { assert(mStatus == ASYNC_DONE); return mResult; }

private:
	ScraperSearchResult mResult;

	typedef std::pair< std::unique_ptr<AsyncHandle>, std::function<void()> > ResolvePair;
	std::vector<ResolvePair> mFuncs;
};

class ImageDownloadHandle : public AsyncHandle
{
public:
	ImageDownloadHandle(const std::string& url, const std::string& path, int maxWidth, int maxHeight);

	void update() override;

private:
	std::unique_ptr<HttpReq> mReq;
	std::string mSavePath;
	int mMaxWidth;
	int mMaxHeight;
};

//About the same as "~/.emulationstation/downloaded_images/[system_name]/[game_name].[url's extension]".
//Will create the "downloaded_images" and "subdirectory" directories if they do not exist.
std::string getSaveAsPath(const ScraperSearchParams& params, const std::string& suffix, const std::string& url);

//Will resize according to Settings::getInt("ScraperResizeWidth") and Settings::getInt("ScraperResizeHeight").
std::unique_ptr<AsyncHandle> downloadImageAsync(const std::string& url, const std::string& saveAs);

// Resolves all metadata assets that need to be downloaded.
std::unique_ptr<MDResolveHandle> resolveMetaDataAssets(const ScraperSearchResult& result, const ScraperSearchParams& search);

//You can pass 0 for maxWidth or maxHeight to automatically keep the aspect ratio.
//Will overwrite the image at [path] with the new resized one.
//Returns true if successful, false otherwise.
bool resizeImage(const std::string& path, int maxWidth, int maxHeight);
