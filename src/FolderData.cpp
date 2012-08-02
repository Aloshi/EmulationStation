#include "FolderData.h"
#include "SystemData.h"
#include <algorithm>

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

	for(unsigned int i = 0; i < name1.length(); i++)
	{
		if(name1[i] != name2[i])
		{
			if(name1[i] < name2[i])
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

void FolderData::sort()
{
	std::sort(mFileVector.begin(), mFileVector.end(), filesort);
}
