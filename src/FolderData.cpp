#include "FolderData.h"
#include "SystemData.h"
#include "GameData.h"
#include <algorithm>
#include <iostream>


std::map<FolderData::ComparisonFunction*, std::string> FolderData::sortStateNameMap;

bool FolderData::isFolder() const { return true; }
const std::string & FolderData::getName() const { return mName; }
const std::string & FolderData::getPath() const { return mPath; }
unsigned int FolderData::getFileCount() { return mFileVector.size(); }


FolderData::FolderData(SystemData* system, std::string path, std::string name)
	: mSystem(system), mPath(path), mName(name)
{
	//first created folder data initializes the list
	if (sortStateNameMap.empty()) {
		sortStateNameMap[compareFileName] = "file name";
		sortStateNameMap[compareRating] = "rating";
		sortStateNameMap[compareUserRating] = "user rating";
		sortStateNameMap[compareTimesPlayed] = "times played";
		sortStateNameMap[compareLastPlayed] = "last time played";
	}
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

//sort this folder and any subfolders
void FolderData::sort(ComparisonFunction & comparisonFunction, bool ascending)
{
	std::sort(mFileVector.begin(), mFileVector.end(), comparisonFunction);

	for(unsigned int i = 0; i < mFileVector.size(); i++)
	{
		if(mFileVector.at(i)->isFolder())
			((FolderData*)mFileVector.at(i))->sort(comparisonFunction, ascending);
	}

	if (!ascending) {
		std::reverse(mFileVector.begin(), mFileVector.end());
	}
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
		return const_cast<GameData*>(game1)->metadata()->getFloat("rating") < const_cast<GameData*>(game2)->metadata()->getFloat("rating");
	}
	return false;
}

bool FolderData::compareUserRating(const FileData* file1, const FileData* file2)
{
	//we need game data. try to cast
	const GameData * game1 = dynamic_cast<const GameData*>(file1);
	const GameData * game2 = dynamic_cast<const GameData*>(file2);
	if (game1 != nullptr && game2 != nullptr) {
		return const_cast<GameData*>(game1)->metadata()->getFloat("userrating") < const_cast<GameData*>(game2)->metadata()->getFloat("userrating");
	}
	return false;
}

bool FolderData::compareTimesPlayed(const FileData* file1, const FileData* file2)
{
	//we need game data. try to cast
	const GameData * game1 = dynamic_cast<const GameData*>(file1);
	const GameData * game2 = dynamic_cast<const GameData*>(file2);
	if (game1 != nullptr && game2 != nullptr) {
		return const_cast<GameData*>(game1)->metadata()->getInt("playcount") < const_cast<GameData*>(game2)->metadata()->getInt("playcount");
	}
	return false;
}

bool FolderData::compareLastPlayed(const FileData* file1, const FileData* file2)
{
	//we need game data. try to cast
	const GameData * game1 = dynamic_cast<const GameData*>(file1);
	const GameData * game2 = dynamic_cast<const GameData*>(file2);
	if (game1 != nullptr && game2 != nullptr) {
		return const_cast<GameData*>(game1)->metadata()->getTime("lastplayed") < const_cast<GameData*>(game2)->metadata()->getTime("lastplayed");
	}
	return false;
}

std::string FolderData::getSortStateName(ComparisonFunction & comparisonFunction, bool ascending)
{
	std::string temp = sortStateNameMap[comparisonFunction];
	if (ascending) {
		temp.append(" (ascending)");
	}
	else {
		temp.append(" (descending)");
	}
	return temp;
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

void FolderData::removeFileRecursive(FileData* f)
{
	auto iter = mFileVector.begin();
	while(iter != mFileVector.end())
	{
		if(*iter == f)
		{
			iter = mFileVector.erase(iter);
		}else{
			
			FolderData* folder = dynamic_cast<FolderData*>(*iter);
			if(folder)
			{
				folder->removeFileRecursive(f);
			}

			iter++;
		}
	}
}
