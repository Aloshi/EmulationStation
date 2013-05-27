#include "FolderData.h"
#include "SystemData.h"
#include <algorithm>
#include <iostream>

bool FolderData::isFolder() { return true; }
std::string FolderData::getName() { return mName; }
std::string FolderData::getPath() { return mPath; }
unsigned int FolderData::getFileCount() { return mFileVector.size(); }
FileData* FolderData::getFile(unsigned int i) { return mFileVector.at(i); }

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
bool filesort(FileData* file1, FileData* file2)
{
	std::string name1 = file1->getName();
	std::string name2 = file2->getName();

	//min of name1/name2 .length()s
	unsigned int count = name1.length() > name2.length() ? name2.length() : name1.length();
	for(unsigned int i = 0; i < count; i++)
	{
		if(toupper(name1[i]) != toupper(name2[i]))
		{
			if(toupper(name1[i]) < toupper(name2[i]))
			{
				return true;
			}else{
				return false;
			}
		}
	}

	if(name1.length() < name2.length())
		return true;
	else
		return false;
}

//sort this folder and any subfolders
void FolderData::sort()
{
	std::sort(mFileVector.begin(), mFileVector.end(), filesort);

	for(unsigned int i = 0; i < mFileVector.size(); i++)
	{
		if(mFileVector.at(i)->isFolder())
			((FolderData*)mFileVector.at(i))->sort();
	}
}
