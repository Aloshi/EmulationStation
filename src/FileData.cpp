#include "FileData.h"
#include <boost/regex/v4/regex.hpp>

namespace fs = boost::filesystem;

std::string getCleanFileName(const fs::path& path)
{
	return regex_replace(path.stem().generic_string(), boost::regex("\\((.*)\\)|\\[(.*)\\]"), "");
}


FileData::FileData(FileType type, const fs::path& path, SystemData* system)
	: mType(type), mPath(path), mSystem(system), mParent(NULL), metadata(type == GAME ? GAME_METADATA : FOLDER_METADATA) // metadata is REALLY set in the constructor!
{
	// metadata needs at least a name field (since that's what getName() will return)
	if(metadata.get("name").empty())
		metadata.set("name", getCleanFileName(mPath));
}

FileData::~FileData()
{
	if(mParent)
		mParent->removeChild(this);

	while(mChildren.size())
		delete mChildren.back();
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
