#include "FileData.h"
#include "SystemData.h"
#include "Log.h"

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


FileData::FileData(FileType type, const fs::path& path, SystemData* system)
	: mType(type), mPath(path), mSystem(system), mParent(NULL), metadata(type == GAME ? GAME_METADATA : FOLDER_METADATA) // metadata is REALLY set in the constructor!
{
	// metadata needs at least a name field (since that's what getName() will return)
	if(metadata.get("name").empty())
		metadata.set("name", getCleanName());
}

FileData::~FileData()
{
	if(mParent)
		mParent->removeChild(this);

	clear();
}

std::string FileData::getCleanName() const
{
	std::string stem = mPath.stem().generic_string();
	if(mSystem && (mSystem->hasPlatformId(PlatformIds::ARCADE) || mSystem->hasPlatformId(PlatformIds::NEOGEO)))
		stem = PlatformIds::getCleanMameName(stem.c_str());
        return stem;
	//return removeParenthesis(stem);
}

const std::string& FileData::getThumbnailPath() const
{
	if(!metadata.get("thumbnail").empty())
		return metadata.get("thumbnail");
	else
		return metadata.get("image");
}


std::vector<FileData*> FileData::getFilesRecursive(unsigned int typeMask) const
{
	std::vector<FileData*> out;

	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if((*it)->getType() & typeMask)
			out.push_back(*it);
		
		if((*it)->getChildren().size() > 0)
		{
			std::vector<FileData*> subchildren = (*it)->getFilesRecursive(typeMask);
			out.insert(out.end(), subchildren.cbegin(), subchildren.cend());
		}
	}

	return out;
}

std::vector<FileData*> FileData::getFavoritesRecursive(unsigned int typeMask) const
{
	std::vector<FileData*> out;
	std::vector<FileData*> files = getFilesRecursive(typeMask);

	for (auto it = files.begin(); it != files.end(); it++)
	{
		if ((*it)->metadata.get("favorite").compare("yes") == 0)
		{
			out.push_back(*it);
		}
	}

	return out;
}

void FileData::changePath(const boost::filesystem::path& path)
{
	clear();

	mPath = path;

	// metadata needs at least a name field (since that's what getName() will return)
	if(metadata.get("name").empty())
		metadata.set("name", getCleanName());
}

void FileData::addChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == NULL);

	mChildren.push_back(file);
	file->mParent = this;
}

void FileData::removeChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == this);

	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if(*it == file)
		{
			mChildren.erase(it);
			return;
		}
	}

	// File somehow wasn't in our children.
	assert(false);
}

void FileData::clear()
{
	while(mChildren.size())
		delete mChildren.back();
}

void FileData::lazyPopulate(const std::vector<std::string>& searchExtensions, SystemData* systemData)
{
	clear();
	populateFolder(this, searchExtensions, systemData);
}

void FileData::sort(ComparisonFunction& comparator, bool ascending)
{
	std::sort(mChildren.begin(), mChildren.end(), comparator);

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

void FileData::populateFolder(FileData* folder, const std::vector<std::string>& searchExtensions, SystemData* systemData)
{
	const fs::path& folderPath = folder->getPath();
	if(!fs::is_directory(folderPath))
	{
		LOG(LogWarning) << "Error - folder with path \"" << folderPath << "\" is not a directory!";
		return;
	}

	const std::string folderStr = folderPath.generic_string();

	//make sure that this isn't a symlink to a thing we already have
	if(fs::is_symlink(folderPath))
	{
		//if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if(folderStr.find(fs::canonical(folderPath).generic_string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << folderPath << "\"";
			return;
		}
	}

	fs::path filePath;
	std::string extension;
	bool isGame;
	for(fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		filePath = (*dir).path();

		if(filePath.stem().empty())
			continue;

		//this is a little complicated because we allow a list of extensions to be defined (delimited with a space)
		//we first get the extension of the file itself:
		extension = filePath.extension().string();

		//fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
		//see issue #75: https://github.com/Aloshi/EmulationStation/issues/75

		isGame = false;
		if((searchExtensions.empty() && !fs::is_directory(filePath)) || (std::find(searchExtensions.begin(), searchExtensions.end(), extension) != searchExtensions.end()
                        && filePath.filename().string().compare(0, 1, ".") != 0)){
			FileData* newGame = new FileData(GAME, filePath.generic_string(), systemData);
			folder->addChild(newGame);
			isGame = true;
		}

		//add directories that also do not match an extension as folders
		if(!isGame && fs::is_directory(filePath))
		{
			FileData* newFolder = new FileData(FOLDER, filePath.generic_string(), systemData);
			folder->addChild(newFolder);
		}
	}
}

void FileData::populateRecursiveFolder(FileData* folder, const std::vector<std::string>& searchExtensions, SystemData* systemData)
{
	const fs::path& folderPath = folder->getPath();
	if(!fs::is_directory(folderPath))
	{
		LOG(LogWarning) << "Error - folder with path \"" << folderPath << "\" is not a directory!";
		return;
	}

	const std::string folderStr = folderPath.generic_string();

	//make sure that this isn't a symlink to a thing we already have
	if(fs::is_symlink(folderPath))
	{
		//if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if(folderStr.find(fs::canonical(folderPath).generic_string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << folderPath << "\"";
			return;
		}
	}

	fs::path filePath;
	std::string extension;
	bool isGame;
	for(fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		filePath = (*dir).path();

		if(filePath.stem().empty())
			continue;

		//this is a little complicated because we allow a list of extensions to be defined (delimited with a space)
		//we first get the extension of the file itself:
		extension = filePath.extension().string();

		//fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
		//see issue #75: https://github.com/Aloshi/EmulationStation/issues/75

		isGame = false;
		if((searchExtensions.empty() && !fs::is_directory(filePath)) || (std::find(searchExtensions.begin(), searchExtensions.end(), extension) != searchExtensions.end()
                        && filePath.filename().string().compare(0, 1, ".") != 0)){
			FileData* newGame = new FileData(GAME, filePath.generic_string(), systemData);
			folder->addChild(newGame);
			isGame = true;
		}

		//add directories that also do not match an extension as folders
		if(!isGame && fs::is_directory(filePath))
		{
			FileData* newFolder = new FileData(FOLDER, filePath.generic_string(), systemData);
			populateRecursiveFolder(newFolder, searchExtensions, systemData);

			//ignore folders that do not contain games
			if(newFolder->getChildren().size() == 0)
				delete newFolder;
			else
				folder->addChild(newFolder);
		}
	}
}
