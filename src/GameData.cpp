#include "GameData.h"
#include <iostream>

bool GameData::isFolder() { return false; }
std::string GameData::getName() { return mName; }

GameData::GameData(SystemData* system, std::string path, std::string name)
{
	mSystem = system;
	mPath = path;
	mName = name;
}

std::string GameData::getPath()
{
	//a quick and dirty way to insert a backslash before most characters that would mess up a bash path
	std::string path = mPath;
	for(unsigned int i = 0; i < path.length(); i++)
	{
		if(path[i] == *" " || path[i] == *"'" || path[i] == *"\"" || path[i] == *"\\")
		{
			path.insert(i, "\\");
			i++;
		}
	}

	return path;
}
