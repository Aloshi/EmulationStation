#include "Scraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../Settings.h"
#include <FreeImage.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "GamesDBScraper.h"
#include "TheArchiveScraper.h"

std::vector<MetaDataList> Scraper::getResults(ScraperSearchParams params)
{
	std::shared_ptr<HttpReq> req = makeHttpReq(params);
	while(req->status() == HttpReq::REQ_IN_PROGRESS);

	return parseReq(params, req);
}

void Scraper::getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc)
{
	std::shared_ptr<HttpReq> httpreq = makeHttpReq(params);
	AsyncReqComponent* req = new AsyncReqComponent(window, httpreq,
		[this, params, returnFunc] (std::shared_ptr<HttpReq> r)
	{
		returnFunc(parseReq(params, r));
	}, [returnFunc] ()
	{
		returnFunc(std::vector<MetaDataList>());
	});

	window->pushGui(req);
}



std::string processFileDownload(std::shared_ptr<HttpReq> r, std::string saveAs)
{
	if(r->status() != HttpReq::REQ_SUCCESS)
	{
		LOG(LogError) << "Failed to download file - HttpReq error: " << r->getErrorMsg();
		return "";
	}

	std::ofstream stream(saveAs, std::ios_base::out | std::ios_base::binary);
	if(stream.fail())
	{
		LOG(LogError) << "Failed to open \"" << saveAs << "\" for writing downloaded file.";
		return "";
	}

	std::string content = r->getContent();
	stream.write(content.data(), content.length());
	stream.close();

	return saveAs;
}

//you can pass 0 for width or height to keep aspect ratio
void resizeImage(const std::string& path, int maxWidth, int maxHeight)
{
	if(maxWidth == 0 && maxHeight == 0)
		return;

	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* image = NULL;
	
	//detect the filetype
	format = FreeImage_GetFileType(path.c_str(), 0);
	if(format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(path.c_str());
	if(format == FIF_UNKNOWN)
	{
		LOG(LogError) << "Error - could not detect filetype for image \"" << path << "\"!";
		return;
	}

	//make sure we can read this filetype first, then load it
	if(FreeImage_FIFSupportsReading(format))
	{
		image = FreeImage_Load(format, path.c_str());
	}else{
		LOG(LogError) << "Error - file format reading not supported for image \"" << path << "\"!";
		return;
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
		return;
	}

	if(!FreeImage_Save(format, imageRescaled, path.c_str()))
	{
		LOG(LogError) << "Failed to save resized image!";
	}

	FreeImage_Unload(imageRescaled);
}

void downloadImageAsync(Window* window, const std::string& url, const std::string& saveAs, std::function<void(std::string)> returnFunc)
{
	std::shared_ptr<HttpReq> httpreq = std::make_shared<HttpReq>(url);
	AsyncReqComponent* req = new AsyncReqComponent(window, httpreq, 
		[returnFunc, saveAs] (std::shared_ptr<HttpReq> r)
	{
		std::string file = processFileDownload(r, saveAs);
		if(!file.empty())
			resizeImage(file, Settings::getInstance()->getInt("ScraperResizeWidth"), Settings::getInstance()->getInt("ScraperResizeHeight"));
		returnFunc(file);
	}, NULL);

	window->pushGui(req);
}

std::string downloadImage(const std::string& url, const std::string& saveAs)
{
	std::shared_ptr<HttpReq> httpreq = std::make_shared<HttpReq>(url);
	while(httpreq->status() == HttpReq::REQ_IN_PROGRESS);

	std::string file = processFileDownload(httpreq, saveAs);

	if(!file.empty())
		resizeImage(file, Settings::getInstance()->getInt("ScraperResizeWidth"), Settings::getInstance()->getInt("ScraperResizeHeight"));

	return file;
}

std::string getSaveAsPath(const ScraperSearchParams& params, const std::string& suffix, const std::string& url)
{
	const std::string subdirectory = params.system->getName();
	const std::string name = params.game->getCleanName() + "-" + suffix;

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


std::shared_ptr<Scraper> createScraperByName(const std::string& name)
{
	if(name == "TheGamesDB")
		return std::shared_ptr<Scraper>(new GamesDBScraper());
	else if(name == "TheArchive")
		return std::shared_ptr<Scraper>(new TheArchiveScraper());

	return nullptr;
}

void resolveMetaDataAssetsAsync(Window* window, const ScraperSearchParams& params, MetaDataList mdl, std::function<void(MetaDataList)> returnFunc)
{
	std::vector<MetaDataDecl> mdd = params.system->getGameMDD();
	for(auto it = mdd.begin(); it != mdd.end(); it++)
	{
		std::string key = it->key;
		std::string val = mdl.get(key);
		if(it->type == MD_IMAGE_PATH && HttpReq::isUrl(val))
		{
			downloadImageAsync(window, val, getSaveAsPath(params, key, val), [window, params, mdl, key, returnFunc] (std::string savedAs) mutable -> 
				void 
			{
				if(savedAs.empty())
				{
					//error
					LOG(LogError) << "Could not resolve image for [" << params.game->getCleanName() << "]! Skipping.";
				}

				mdl.set(key, savedAs);
				resolveMetaDataAssetsAsync(window, params, mdl, returnFunc);
			});
			return;
		}
	}

	returnFunc(mdl);
}
