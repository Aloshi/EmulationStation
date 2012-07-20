#include "GameData.h"

GameData::GameData(SystemData* system, std::string path, std::string name)
{
	mSystem = system;
	mPath = path;
	mName = name;
}

std::string GameData::getName()
{
	return mName;
}
