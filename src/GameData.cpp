#include "GameData.h"
#include <boost/filesystem.hpp>
#include <iostream>

bool GameData::isFolder() { return false; }
std::string GameData::getName() { return mName; }
std::string GameData::getPath() { return mPath; }
std::string GameData::getDescription() { return mDescription; }
std::string GameData::getImagePath() { return mImagePath; }

GameData::GameData(SystemData* system, std::string path, std::string name)
{
	mSystem = system;
	mPath = path;
	mName = name;

	mDescription = "";
	mImagePath = "";
}

std::string GameData::getBashPath()
{
	//a quick and dirty way to insert a backslash before most characters that would mess up a bash path
	std::string path = mPath;
	const char* invalidChars = " '\"\\!$^&*(){}[]?;<>";
	for(unsigned int i = 0; i < path.length(); i++)
	{
		char c;
		unsigned int charNum = 0;
		do {
			c = invalidChars[charNum];
			if(path[i] == c)
			{
				path.insert(i, "\\");
				i++;
				break;
			}
			charNum++;
		} while(c != '\0');
	}

	return path;
}

void GameData::set(std::string name, std::string description, std::string imagePath)
{
	if(!name.empty())
		mName = name;
	if(!description.empty())
		mDescription = description;
	if(!imagePath.empty() && boost::filesystem::exists(imagePath))
		mImagePath = imagePath;
}
