#include "SystemData.h"
#include "GameData.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

SystemData::SystemData(std::string name, std::string startPath, std::string extension)
{
	mName = name;
	mStartPath = startPath;
	mSearchExtension = extension;
	buildGameList();
}

SystemData::~SystemData()
{
	for(unsigned int i = 0; i < mGameVector.size(); i++)
	{
		delete mGameVector.at(i);
	}
	mGameVector.clear();
}



void SystemData::buildGameList()
{
	std::cout << "System " << mName << " building game list...\n";

	if(!fs::is_directory(mStartPath))
	{
		std::cout << "Error - system \"" << mName << "\"'s start path does not exist!\n";
		return;
	}

	for(fs::recursive_directory_iterator end, dir(mStartPath); dir != end; ++dir)
	{
		std::cout << "File found: " << *dir << "\n";

		fs::path path = (*dir).path();

		if(fs::is_directory(path))
			continue;

		std::string name = path.stem().string();
		std::string extension = path.extension().string();

		std::cout << "detected name as \"" << name <<"\", extension as \"" << extension << "\"\n";

		if(extension == mSearchExtension)
		{
			mGameVector.push_back(new GameData(this, path.string(), name));
			std::cout << "Added game \"" << name << "\"\n";
		}
	}

	std::cout << "...done! Found " << mGameVector.size() << " games.\n";
}

unsigned int SystemData::getGameCount()
{
	return mGameVector.size();
}

GameData* SystemData::getGame(unsigned int i)
{
	return mGameVector.at(i);
}

std::string SystemData::getName()
{
	return mName;
}
