#ifndef _FILEDATA_H_
#define _FILEDATA_H_

#include <string>

//This is a really basic class that the GameData and FolderData subclass from.
//This lets us keep everything in one vector and not have to differentiate between files and folders when we just want to check the name, etc.
class FileData
{
public:
	virtual ~FileData() { };
	virtual bool isFolder() = 0;
	virtual std::string getName() = 0;
	virtual std::string getPath() = 0;
};

#endif
