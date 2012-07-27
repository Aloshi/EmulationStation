#include "FolderData.h"
#include "SystemData.h"

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

