#include "GameData.h"
#include <iostream>

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



std::string GameData::getValidPath()
{
	//a quick and dirty way to insert a backslash before spaces
	std::string path = mPath;
	for(unsigned int i = 0; i < path.length(); i++)
	{
		if(path[i] == *" ")
		{
			path.insert(i, "\\");
			i++;
		}
	}

	return path;
}
