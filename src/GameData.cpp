#include "GameData.h"
#include <boost/filesystem.hpp>
#include <boost/regex/v4/regex.hpp>
#include <iostream>
#include <ctime>
#include <sstream>

GameData::GameData(const std::string& path, const MetaDataList& metadata)
	: mPath(path), mBaseName(boost::filesystem::path(path).stem().string()), mMetaData(metadata)
{
	if(mMetaData.get("name").empty())
		mMetaData.set("name", mBaseName);
}

bool GameData::isFolder() const
{
	return false;
}

boost::posix_time::ptime GameData::isSelected() const
{
    return const_cast<GameData*>(this)->metadata()->getTime("selected");
}

void GameData::setSelected(bool isSelected)
{
    if (isSelected)
    {
        metadata()->setTime("selected", boost::posix_time::second_clock::universal_time());
    }
    else
    {
        metadata()->set("selected", "");
    }
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

std::string GameData::getCleanName() const
{
	return regex_replace(mBaseName, boost::regex("\\((.*)\\)|\\[(.*)\\]"), "");
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
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	metadata()->setTime("lastplayed", time);
}

MetaDataList* GameData::metadata()
{
	return &mMetaData;
}
