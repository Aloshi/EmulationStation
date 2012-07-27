#ifndef _FOLDER_H_
#define _FOLDER_H_

#include "FileData.h"
#include <vector>

class SystemData;

class FolderData : public FileData
{
public:
	FolderData(SystemData* system, std::string path, std::string name);
	~FolderData();

	bool isFolder();
	std::string getName();
	std::string getPath();

	unsigned int getFileCount();
	FileData* getFile(unsigned int i);

	void pushFileData(FileData* file);
private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;
	std::vector<FileData*> mFileVector;
};

#endif
