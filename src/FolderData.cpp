#include "FolderData.h"
#include "SystemData.h"
#include "GameData.h"
#include <algorithm>
#include <iostream>

bool FolderData::isFolder() const { return true; }
const std::string & FolderData::getName() const { return mName; }
const std::string & FolderData::getPath() const { return mPath; }
unsigned int FolderData::getFileCount() { return mFileVector.size(); }


FolderData::FolderData(SystemData* system, std::string path, std::string name)
{
	mSystem = system;
	mPath = path;
	mName = name;
}

FolderData::~FolderData()
{
	for(unsigned int i = 0; i < mFileVector.size(); i++)
	{
		delete mFileVector.at(i);
	}

	mFileVector.clear();
}

void FolderData::pushFileData(FileData* file)
{
	mFileVector.push_back(file);
}

//returns if file1 should come before file2
bool FolderData::compareFileName(const FileData* file1, const FileData* file2)
{
	std::string name1 = file1->getName();
	std::string name2 = file2->getName();

	//min of name1/name2 .length()s
	unsigned int count = name1.length() > name2.length() ? name2.length() : name1.length();
	for(unsigned int i = 0; i < count; i++)
	{
		if(toupper(name1[i]) != toupper(name2[i]))
		{
			return toupper(name1[i]) < toupper(name2[i]);
		}
	}

	return name1.length() < name2.length();
}

bool FolderData::compareRating(const FileData* file1, const FileData* file2)
{
	//we need game data. try to cast
	const GameData * game1 = dynamic_cast<const GameData*>(file1);
	const GameData * game2 = dynamic_cast<const GameData*>(file2);
	if (game1 != nullptr && game2 != nullptr) {
		return game1->getRating() < game2->getRating();
	}
	return false;
}

bool FolderData::compareTimesPlayed(const FileData* file1, const FileData* file2)
{
	//we need game data. try to cast
	const GameData * game1 = dynamic_cast<const GameData*>(file1);
	const GameData * game2 = dynamic_cast<const GameData*>(file2);
	if (game1 != nullptr && game2 != nullptr) {
		return game1->getTimesPlayed() < game2->getTimesPlayed();
	}
	return false;
}

//sort this folder and any subfolders
void FolderData::sort()
{
	std::sort(mFileVector.begin(), mFileVector.end(), compareFileName);

	for(unsigned int i = 0; i < mFileVector.size(); i++)
	{
		if(mFileVector.at(i)->isFolder())
			((FolderData*)mFileVector.at(i))->sort();
	}
}

FileData* FolderData::getFile(unsigned int i) const
{
	return mFileVector.at(i);
}

std::vector<FileData*> FolderData::getFiles(bool onlyFiles) const
{
	std::vector<FileData*> temp;
	//now check if a child is a folder and get those children in turn
	std::vector<FileData*>::const_iterator fdit = mFileVector.cbegin();
	while(fdit != mFileVector.cend()) {
		//dynamically try to cast to FolderData type
		FolderData * folder = dynamic_cast<FolderData*>(*fdit);
		if (folder != nullptr) {
			//add this only when user wanted it
			if (!onlyFiles) {
				temp.push_back(*fdit);
			}
		}
		else {
			temp.push_back(*fdit);
		}
		++fdit;
	}
	return temp;
}

std::vector<FileData*> FolderData::getFilesRecursive(bool onlyFiles) const
{
	std::vector<FileData*> temp;
	//now check if a child is a folder and get those children in turn
	std::vector<FileData*>::const_iterator fdit = mFileVector.cbegin();
	while(fdit != mFileVector.cend()) {
		//dynamically try to cast to FolderData type
		FolderData * folder = dynamic_cast<FolderData*>(*fdit);
		if (folder != nullptr) {
			//add this onyl when user wanted it
			if (!onlyFiles) {
				temp.push_back(*fdit);
			}
			//worked. Is actual folder data. recurse
			std::vector<FileData*> children = folder->getFilesRecursive(onlyFiles);
			//insert children into return vector
			temp.insert(temp.end(), children.cbegin(), children.cend());
		}
		else {
			temp.push_back(*fdit);
		}
		++fdit;
	}
	return temp;
}