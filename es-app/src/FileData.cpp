#include "FileData.h"

#include "AudioManager.h"
#include "CollectionSystemManager.h"
#include "FileFilterIndex.h"
#include "FileSorts.h"
#include "Log.h"
#include "platform.h"
#include "SystemData.h"
#include "Util.h"
#include "VolumeControl.h"
#include "Window.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

FileData::FileData(FileType type, const fs::path& path, SystemEnvironmentData* envData, SystemData* system)
	: mType(type), mPath(path), mSystem(system), mEnvData(envData), mSourceFileData(NULL), mParent(NULL), metadata(type == GAME ? GAME_METADATA : FOLDER_METADATA) // metadata is REALLY set in the constructor!
{
	// metadata needs at least a name field (since that's what getName() will return)
	if(metadata.get("name").empty())
		metadata.set("name", getDisplayName());
	mSystemName = system->getName();
}

FileData::~FileData()
{
	if(mParent)
		mParent->removeChild(this);

	if(mType == GAME)
		mSystem->getIndex()->removeFromIndex(this);

	mChildren.clear();
}

std::string FileData::getDisplayName() const
{
	std::string stem = mPath.stem().generic_string();
	if(mSystem && mSystem->hasPlatformId(PlatformIds::ARCADE) || mSystem->hasPlatformId(PlatformIds::NEOGEO))
		stem = PlatformIds::getCleanMameName(stem.c_str());

	return stem;
}

std::string FileData::getCleanName() const
{
	return removeParenthesis(this->getDisplayName());
}

const std::string FileData::getThumbnailPath() const
{
	std::string thumbnail = metadata.get("thumbnail");

	// no thumbnail, try image
	if(thumbnail.empty())
	{
		thumbnail = metadata.get("image");

		// no image, try to use local image
		if(thumbnail.empty())
		{
			const char* extList[2] = { ".png", ".jpg" };
			for(int i = 0; i < 2; i++)
			{
				if(thumbnail.empty())
				{
					std::string path = mEnvData->mStartPath + "/images/" + getDisplayName() + "-image" + extList[i];
					if(boost::filesystem::exists(path))
						thumbnail = path;
				}
			}
		}
	}

	return thumbnail;
}

const std::string& FileData::getName()
{
	return metadata.get("name");
}

const std::vector<FileData*>& FileData::getChildrenListToDisplay() {

	FileFilterIndex* idx = CollectionSystemManager::get()->getSystemToView(mSystem)->getIndex();
	if (idx->isFiltered()) {
		mFilteredChildren.clear();
		for(auto it = mChildren.begin(); it != mChildren.end(); it++)
		{
			if (idx->showFile((*it))) {
				mFilteredChildren.push_back(*it);
			}
		}

		return mFilteredChildren;
	}
	else
	{
		return mChildren;
	}
}

const std::string FileData::getVideoPath() const
{
	std::string video = metadata.get("video");

	// no video, try to use local video
	if(video.empty())
	{
		std::string path = mEnvData->mStartPath + "/images/" + getDisplayName() + "-video.mp4";
		if(boost::filesystem::exists(path))
			video = path;
	}

	return video;
}

const std::string FileData::getMarqueePath() const
{
	std::string marquee = metadata.get("marquee");

	// no marquee, try to use local marquee
	if(marquee.empty())
	{
		const char* extList[2] = { ".png", ".jpg" };
		for(int i = 0; i < 2; i++)
		{
			if(marquee.empty())
			{
				std::string path = mEnvData->mStartPath + "/images/" + getDisplayName() + "-marquee" + extList[i];
				if(boost::filesystem::exists(path))
					marquee = path;
			}
		}
	}

	return marquee;
}

const std::string FileData::getImagePath() const
{
	std::string image = metadata.get("image");

	// no image, try to use local image
	if(image.empty())
	{
		const char* extList[2] = { ".png", ".jpg" };
		for(int i = 0; i < 2; i++)
		{
			if(image.empty())
			{
				std::string path = mEnvData->mStartPath + "/images/" + getDisplayName() + "-image" + extList[i];
				if(boost::filesystem::exists(path))
					image = path;
			}
		}
	}

	return image;
}

std::vector<FileData*> FileData::getFilesRecursive(unsigned int typeMask, bool displayedOnly) const
{
	std::vector<FileData*> out;
	FileFilterIndex* idx = mSystem->getIndex();

	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if((*it)->getType() & typeMask)
		{
			if (!displayedOnly || !idx->isFiltered() || idx->showFile(*it))
				out.push_back(*it);
		}

		if((*it)->getChildren().size() > 0)
		{
			std::vector<FileData*> subchildren = (*it)->getFilesRecursive(typeMask, displayedOnly);
			out.insert(out.end(), subchildren.cbegin(), subchildren.cend());
		}
	}

	return out;
}

std::string FileData::getKey() {
	return getFileName();
}

FileData* FileData::getSourceFileData()
{
	return this;
}

void FileData::addChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == NULL);

	const std::string key = file->getKey();
	if (mChildrenByFilename.find(key) == mChildrenByFilename.end())
	{
		mChildrenByFilename[key] = file;
		mChildren.push_back(file);
		file->mParent = this;
	}
}

void FileData::removeChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == this);
	mChildrenByFilename.erase(file->getKey());
	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if(*it == file)
		{
			file->mParent = NULL;
			mChildren.erase(it);
			return;
		}
	}

	// File somehow wasn't in our children.
	assert(false);

}

void FileData::sort(ComparisonFunction& comparator, bool ascending)
{
	std::stable_sort(mChildren.begin(), mChildren.end(), comparator);

	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if((*it)->getChildren().size() > 0)
			(*it)->sort(comparator, ascending);
	}

	if(!ascending)
		std::reverse(mChildren.begin(), mChildren.end());
}

void FileData::sort(const SortType& type)
{
	sort(*type.comparisonFunction, type.ascending);
}

void FileData::launchGame(Window* window)
{
	LOG(LogInfo) << "Attempting to launch game...";

	AudioManager::getInstance()->deinit();
	VolumeControl::getInstance()->deinit();
	window->deinit();

	std::string command = mEnvData->mLaunchCommand;

	const std::string rom = escapePath(getPath());
	const std::string basename = getPath().stem().string();
	const std::string rom_raw = fs::path(getPath()).make_preferred().string();

	command = strreplace(command, "%ROM%", rom);
	command = strreplace(command, "%BASENAME%", basename);
	command = strreplace(command, "%ROM_RAW%", rom_raw);

	LOG(LogInfo) << "	" << command;
	int exitCode = runSystemCommand(command);

	if(exitCode != 0)
	{
		LOG(LogWarning) << "...launch terminated with nonzero exit code " << exitCode << "!";
	}

	window->init();
	VolumeControl::getInstance()->init();
	window->normalizeNextUpdate();

	//update number of times the game has been launched

	FileData* gameToUpdate = getSourceFileData();

	int timesPlayed = gameToUpdate->metadata.getInt("playcount") + 1;
	gameToUpdate->metadata.set("playcount", std::to_string(static_cast<long long>(timesPlayed)));

	//update last played time
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	gameToUpdate->metadata.setTime("lastplayed", time);
	CollectionSystemManager::get()->refreshCollectionSystems(gameToUpdate);
}

CollectionFileData::CollectionFileData(FileData* file, SystemData* system)
	: FileData(file->getSourceFileData()->getType(), file->getSourceFileData()->getPath(), file->getSourceFileData()->getSystemEnvData(), system)
{
	// we use this constructor to create a clone of the filedata, and change its system
	mSourceFileData = file->getSourceFileData();
	refreshMetadata();
	mParent = NULL;
	metadata = mSourceFileData->metadata;
	mSystemName = mSourceFileData->getSystem()->getName();
}

CollectionFileData::~CollectionFileData()
{
	// need to remove collection file data at the collection object destructor
	if(mParent)
		mParent->removeChild(this);
	mParent = NULL;
}

std::string CollectionFileData::getKey() {
	return getFullPath();
}

FileData* CollectionFileData::getSourceFileData()
{
	return mSourceFileData;
}

void CollectionFileData::refreshMetadata()
{
	metadata = mSourceFileData->metadata;
	mDirty = true;
}

const std::string& CollectionFileData::getName()
{
	if (mDirty) {
		mCollectionFileName = removeParenthesis(mSourceFileData->metadata.get("name"));
		boost::trim(mCollectionFileName);
		mCollectionFileName += " [" + strToUpper(mSourceFileData->getSystem()->getName()) + "]";
		mDirty = false;
	}
	return mCollectionFileName;
}

// returns Sort Type based on a string description
FileData::SortType getSortTypeFromString(std::string desc) {
	std::vector<FileData::SortType> SortTypes = FileSorts::SortTypes;
	// find it
	for(unsigned int i = 0; i < FileSorts::SortTypes.size(); i++)
	{
		const FileData::SortType& sort = FileSorts::SortTypes.at(i);
		if(sort.description == desc)
		{
			return sort;
		}
	}
	// if not found default to name, ascending
	return FileSorts::SortTypes.at(0);
}
