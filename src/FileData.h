#ifndef _FILEDATA_H_
#define _FILEDATA_H_

#include <string>

class FileData
{
public:
	virtual bool isFolder() = 0;
	virtual std::string getName() = 0;
	virtual std::string getPath() = 0;
};

#endif
