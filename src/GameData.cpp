#include "GameData.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <ctime>
#include <sstream>

GameData::GameData(SystemData* system, std::string path)
	: mSystem(system), mPath(path), mBaseName(boost::filesystem::path(path).stem().string()), mMetaData(MetaDataList::getDefaultGameMDD())
{
	if(mMetaData.get("name").empty())
		mMetaData.set("name", mBaseName);
}

bool GameData::isFolder() const
{
	return false;
}

const std::string& GameData::getName() const
{
	return mMetaData.get("name");
}

const std::string& GameData::getPath() const
{
	return mPath;
}

std::string GameData::getBashPath() const
{
	//a quick and dirty way to insert a backslash before most characters that would mess up a bash path
	std::string path = getPath();

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

//returns the boost::filesystem stem of our path - e.g. for "/foo/bar.rom" returns "bar"
std::string GameData::getBaseName() const
{
	return mBaseName;
}

void GameData::incTimesPlayed()
{
	int timesPlayed = metadata()->getInt("playcount");
	timesPlayed++;

	std::stringstream ss;
	metadata()->set("playcount", std::to_string(static_cast<long long>(timesPlayed)));
}

void GameData::lastPlayedNow()
{
	std::stringstream ss;
	ss << std::time(nullptr);
	metadata()->set("lastplayed", ss.str());
}

MetaDataList* GameData::metadata()
{
	return &mMetaData;
}
