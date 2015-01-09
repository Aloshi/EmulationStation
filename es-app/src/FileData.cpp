#include "FileData.h"
#include "SystemData.h"
#include "SystemManager.h"
#include "Settings.h"

namespace fs = boost::filesystem;

std::string removeParenthesis(const std::string& str)
{
	// remove anything in parenthesis or brackets
	// should be roughly equivalent to the regex replace "\((.*)\)|\[(.*)\]" with ""
	// I would love to just use regex, but it's not worth pulling in another boost lib for one function that is used once

	std::string ret = str;
	size_t start, end;

	static const int NUM_TO_REPLACE = 2;
	static const char toReplace[NUM_TO_REPLACE*2] = { '(', ')', '[', ']' };

	bool done = false;
	while(!done)
	{
		done = true;
		for(int i = 0; i < NUM_TO_REPLACE; i++)
		{
			end = ret.find_first_of(toReplace[i*2+1]);
			start = ret.find_last_of(toReplace[i*2], end);

			if(start != std::string::npos && end != std::string::npos)
			{
				ret.erase(start, end - start + 1);
				done = false;
			}
		}
	}

	// also strip whitespace
	end = ret.find_last_not_of(' ');
	if(end != std::string::npos)
		end++;

	ret = ret.substr(0, end);

	return ret;
}

std::string getCleanGameName(const std::string& str, const SystemData* system)
{
	fs::path path(str);
	std::string stem = path.stem().generic_string();
	if(system && system->hasPlatformId(PlatformIds::ARCADE) || system->hasPlatformId(PlatformIds::NEOGEO))
		stem = PlatformIds::getCleanMameName(stem.c_str());

	return removeParenthesis(stem);
}

FileData::FileData(const std::string& fileID, SystemData* system, FileType type, const std::string& nameCache)
	: mFileID(fileID), mSystem(system), mType(type), mNameCache(nameCache)
{
}

FileData::FileData() : FileData("", NULL, (FileType)0)
{
}

FileData::FileData(const std::string& fileID, const std::string& systemID, FileType type) : 
	FileData(fileID, SystemManager::getInstance()->getSystemByName(systemID), type)
{
}

const std::string& FileData::getSystemID() const
{
	return mSystem->getName();
}

const std::string& FileData::getName() const
{
	// try and cache what's in the DB
	if(mNameCache.empty())
		mNameCache = get_metadata().get<std::string>("name");

	// nothing was in the DB...use the clean version of our path
	if(mNameCache.empty())
		mNameCache = getCleanName();

	return mNameCache;
}

fs::path FileData::getPath() const
{
	return fileIDToPath(mFileID, mSystem);
}

FileType FileData::getType() const
{
	return mType;
}

MetaDataMap FileData::get_metadata() const
{
	return SystemManager::getInstance()->database().getFileData(mFileID, mSystem->getName());
}

void FileData::set_metadata(const MetaDataMap& metadata)
{
	SystemManager::getInstance()->database().setFileData(mFileID, getSystemID(), mType, metadata);
}

std::vector<FileData> FileData::getChildren(const FileSort* sort) const
{
	if(sort == NULL)
		sort = &getFileSorts().at(Settings::getInstance()->getInt("SortTypeIndex"));

	return SystemManager::getInstance()->database().getChildrenOf(mFileID, mSystem, true, true, sort);
}

std::vector<FileData> FileData::getChildrenRecursive(bool includeFolders, const FileSort* sort) const
{
	if(sort == NULL)
		sort = &getFileSorts().at(Settings::getInstance()->getInt("SortTypeIndex"));

	return SystemManager::getInstance()->database().getChildrenOf(mFileID, mSystem, false, includeFolders, sort);
}
