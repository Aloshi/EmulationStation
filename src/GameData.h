#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include <string>
#include "FileData.h"
#include "SystemData.h"

class GameData : public FileData
{
public:
	GameData(SystemData* system, std::string path, std::string name);

	std::string getName();
	std::string getPath();
	bool isFolder();
private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;
};

#endif
