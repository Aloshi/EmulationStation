#include "FileData.h"
#include "SystemData.h"

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
		metadata.set("name", getDisplayName());
}

FileData::~FileData()
{
	if(mParent)
		mParent->removeChild(this);

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

const std::string& FileData::getThumbnailPath() const
{
	if(!metadata.get("thumbnail").empty())
		return metadata.get("thumbnail");
	else
		return metadata.get("image");
}

const std::vector<FileData*>& FileData::getChildrenListToDisplay() {

	FileFilterIndex* idx = mSystem->getIndex();
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

const std::string& FileData::getVideoPath() const
{
	return metadata.get("video");
}

const std::string& FileData::getMarqueePath() const
{
	return metadata.get("marquee");
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

void FileData::addChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == NULL);

	const std::string key = file->getPath().filename().string();
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
	mChildrenByFilename.erase(file->getPath().filename().string());
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
