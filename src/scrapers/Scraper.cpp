#include "Scraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../Settings.h"
#include <FreeImage.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "GamesDBScraper.h"
#include "TheArchiveScraper.h"

std::shared_ptr<Scraper> createScraperByName(const std::string& name)
{
	if(name == "TheGamesDB")
		return std::shared_ptr<Scraper>(new GamesDBScraper());
	else if(name == "TheArchive")
		return std::shared_ptr<Scraper>(new TheArchiveScraper());

	return nullptr;
}

std::unique_ptr<MDResolveHandle> resolveMetaDataAssets(const ScraperSearchResult& result, const ScraperSearchParams& search)
{
	return std::unique_ptr<MDResolveHandle>(new MDResolveHandle(result, search));
}

MDResolveHandle::MDResolveHandle(const ScraperSearchResult& result, const ScraperSearchParams& search) : mResult(result)
{
	if(!result.imageUrl.empty())
	{
		std::string imgPath = getSaveAsPath(search, "image", result.imageUrl);
		mFuncs.push_back(ResolvePair(downloadImageAsync(result.imageUrl, imgPath), [this, imgPath]
		{
			mResult.mdl.set("image", imgPath);
			mResult.imageUrl = "";
		}));
	}
}

void MDResolveHandle::update()
{
	if(mStatus == ASYNC_DONE || mStatus == ASYNC_ERROR)
		return;
	
	auto it = mFuncs.begin();
	while(it != mFuncs.end())
	{
		if(it->first->status() == ASYNC_ERROR)
		{
			setError(it->first->getStatusString());
			return;
		}else if(it->first->status() == ASYNC_DONE)
		{
			it->second();
			it = mFuncs.erase(it);
			continue;
		}
		it++;
	}

	if(mFuncs.empty())
		setStatus(ASYNC_DONE);
}

std::unique_ptr<AsyncHandle> downloadImageAsync(const std::string& url, const std::string& saveAs)
{
	return std::unique_ptr<ImageDownloadHandle>(new ImageDownloadHandle(url, saveAs, 
		Settings::getInstance()->getInt("ScraperResizeWidth"), Settings::getInstance()->getInt("ScraperResizeHeight")));
}

ImageDownloadHandle::ImageDownloadHandle(const std::string& url, const std::string& path, int maxWidth, int maxHeight) : 
	mSavePath(path), mMaxWidth(maxWidth), mMaxHeight(maxHeight), mReq(new HttpReq(url))
{
}

void ImageDownloadHandle::update()
{
	if(mReq->status() == HttpReq::REQ_IN_PROGRESS)
		return;

	if(mReq->status() != HttpReq::REQ_SUCCESS)
	{
		std::stringstream ss;
		ss << "Network error: " << mReq->getErrorMsg();
		setError(ss.str());
		return;
	}

	// download is done, save it to disk
	std::ofstream stream(mSavePath, std::ios_base::out | std::ios_base::binary);
	if(stream.bad())
	{
		setError("Failed to open image path to write. Permission error? Disk full?");
		return;
	}

	const std::string& content = mReq->getContent();
	stream.write(content.data(), content.length());
	stream.close();
	if(stream.bad())
	{
		setError("Failed to save image. Disk full?");
		return;
	}

	// resize it
	if(!resizeImage(mSavePath, mMaxWidth, mMaxHeight))
	{
		setError("Error saving resized image. Out of memory? Disk full?");
		return;
	}

	setStatus(ASYNC_DONE);
}

//you can pass 0 for width or height to keep aspect ratio
bool resizeImage(const std::string& path, int maxWidth, int maxHeight)
{
	// nothing to do
	if(maxWidth == 0 && maxHeight == 0)
		return true;

	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* image = NULL;
	
	//detect the filetype
	format = FreeImage_GetFileType(path.c_str(), 0);
	if(format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(path.c_str());
	if(format == FIF_UNKNOWN)
	{
		LOG(LogError) << "Error - could not detect filetype for image \"" << path << "\"!";
		return false;
	}

	//make sure we can read this filetype first, then load it
	if(FreeImage_FIFSupportsReading(format))
	{
		image = FreeImage_Load(format, path.c_str());
	}else{
		LOG(LogError) << "Error - file format reading not supported for image \"" << path << "\"!";
		return false;
	}

	float width = (float)FreeImage_GetWidth(image);
	float height = (float)FreeImage_GetHeight(image);

	if(maxWidth == 0)
	{
		maxWidth = (int)((maxHeight / height) * width);
	}else if(maxHeight == 0)
	{
		maxHeight = (int)((maxWidth / width) * height);
	}

	FIBITMAP* imageRescaled = FreeImage_Rescale(image, maxWidth, maxHeight, FILTER_BILINEAR);
	FreeImage_Unload(image);

	if(imageRescaled == NULL)
	{
		LOG(LogError) << "Could not resize image! (not enough memory? invalid bitdepth?)";
		return false;
	}

	bool saved = FreeImage_Save(format, imageRescaled, path.c_str());
	FreeImage_Unload(imageRescaled);

	if(!saved)
		LOG(LogError) << "Failed to save resized image!";

	return saved;
}

std::string getSaveAsPath(const ScraperSearchParams& params, const std::string& suffix, const std::string& url)
{
	const std::string subdirectory = params.system->getName();
	const std::string name = getCleanFileName(params.game->getPath()) + "-" + suffix;

	std::string path = getHomePath() + "/.emulationstation/downloaded_images/";

	if(!boost::filesystem::exists(path))
		boost::filesystem::create_directory(path);

	path += subdirectory + "/";

	if(!boost::filesystem::exists(path))
		boost::filesystem::create_directory(path);

	size_t dot = url.find_last_of('.');
	std::string ext;
	if(dot != std::string::npos)
		ext = url.substr(dot, std::string::npos);

	path += name + ext;
	return path;
}
