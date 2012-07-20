#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include <string>
#include "SystemData.h"

class GameData
{
public:
	GameData(SystemData* system, std::string path, std::string name);

	std::string getName();
private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;
};

#endif
