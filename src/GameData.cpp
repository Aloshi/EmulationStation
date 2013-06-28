#include "GameData.h"
#include <boost/filesystem.hpp>
#include <iostream>


const std::string GameData::xmlTagGameList = "gameList";
const std::string GameData::xmlTagGame = "game";
const std::string GameData::xmlTagName = "name";
const std::string GameData::xmlTagPath = "path";
const std::string GameData::xmlTagDescription = "desc";
const std::string GameData::xmlTagImagePath = "image";
const std::string GameData::xmlTagRating = "rating";
const std::string GameData::xmlTagUserRating = "userrating";
const std::string GameData::xmlTagTimesPlayed = "timesplayed";
const std::string GameData::xmlTagLastPlayed = "lastplayed";


GameData::GameData(SystemData* system, std::string path, std::string name)
	: mSystem(system), mPath(path), mName(name), mRating(0.0f), mUserRating(0.0f), mTimesPlayed(0), mLastPlayed(0)
{
}

bool GameData::isFolder() const
{
	return false;
}

const std::string & GameData::getName() const
{
	return mName;
}

void GameData::setName(const std::string & name)
{
	mName = name;
}

const std::string & GameData::getPath() const
{
	return mPath;
}

void GameData::setPath(const std::string & path)
{
	mPath = path;
}

const std::string & GameData::getDescription() const
{
	return mDescription;
}

void GameData::setDescription(const std::string & description)
{
	mDescription = description;
}

const std::string & GameData::getImagePath() const
{
	return mImagePath;
}

void GameData::setImagePath(const std::string & imagePath)
{
	mImagePath = imagePath;
}

float GameData::getRating() const
{
	return mRating;
}

void GameData::setRating(float rating)
{
	mRating = rating;
}

float GameData::getUserRating() const
{
	return mUserRating;
}

void GameData::setUserRating(float rating)
{
	mUserRating = rating;
}

size_t GameData::getTimesPlayed() const
{
	return mTimesPlayed;
}

void GameData::setTimesPlayed(size_t timesPlayed)
{
	mTimesPlayed = timesPlayed;
}

std::time_t GameData::getLastPlayed() const
{
	return mLastPlayed;
}

void GameData::setLastPlayed(std::time_t lastPlayed)
{
	mLastPlayed = lastPlayed;
}

std::string GameData::getBashPath() const
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

//returns the boost::filesystem stem of our path - e.g. for "/foo/bar.rom" returns "bar"
std::string GameData::getBaseName() const
{
	boost::filesystem::path path(mPath);
	return path.stem().string();
}
