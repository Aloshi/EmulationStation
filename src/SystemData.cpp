#include "SystemData.h"
#include "GameData.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>

namespace fs = boost::filesystem;

SystemData::SystemData(std::string name, std::string startPath, std::string extension, std::string command)
{
	mName = name;
	mStartPath = startPath;
	mSearchExtension = extension;
	mLaunchCommand = command;
	buildGameList();
}

SystemData::~SystemData()
{
	deleteGames();
}

std::string strreplace(std::string& str, std::string replace, std::string with)
{
	size_t pos = str.find(replace);

	return str.replace(pos, replace.length(), with.c_str(), with.length());
}

void SystemData::launchGame(unsigned int i)
{
	std::cout << "Attempting to launch game...\n";

	std::string command = mLaunchCommand;
	GameData* game = mGameVector.at(i);

	command = strreplace(command, "%ROM%", game->getValidPath());

	std::cout << "	" << command << "\n";
	std::cout << "=====================================================\n";
	system(command.c_str());
	std::cout << "=====================================================\n";

	std::cout << "...launch terminated!\n";
}

void SystemData::deleteGames()
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

	deleteGames();

	if(!fs::is_directory(mStartPath))
	{
		std::cout << "Error - system \"" << mName << "\"'s start path does not exist!\n";
		return;
	}

	for(fs::recursive_directory_iterator end, dir(mStartPath); dir != end; ++dir)
	{
		//std::cout << "File found: " << *dir << "\n";

		fs::path path = (*dir).path();

		if(fs::is_directory(path))
			continue;

		std::string name = path.stem().string();
		std::string extension = path.extension().string();

		if(extension == mSearchExtension)
		{
			mGameVector.push_back(new GameData(this, path.string(), name));
			std::cout << "	Added game \"" << name << "\"\n";
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

//creates system files from information located in a config file
std::vector<SystemData*> SystemData::loadConfig(std::string path)
{
	std::cout << "Loading system config file \"" << path << "\"...\n";

	std::vector<SystemData*> returnVec;

	std::ifstream file(path.c_str());

	if(file.is_open())
	{
		std::string line;
		std::string sysName, sysPath, sysExtension, sysCommand;
		while(file.good())
		{
			std::getline(file, line);

			//skip blank lines
			if(line.empty())
				continue;

			//find the name (left of the equals sign) and the value (right of the equals sign)
			bool lineValid = false;
			std::string varName, varValue;
			for(unsigned int i = 0; i < line.length(); i++)
			{
				if(line[i] == *"=")
				{
					lineValid = true;
					varName = line.substr(0, i);
					varValue = line.substr(i + 1, line.length() - 1);
					std::cout << "	" << varName << " = " << varValue << "\n";
					break;
				}
			}

			if(lineValid)
			{
				if(varName == "NAME")
					sysName = varValue;
				else if(varName == "PATH")
					sysPath = varValue;
				else if(varName == "EXTENSION")
					sysExtension = varValue;
				else if(varName == "COMMAND")
					sysCommand = varValue;
				else
					std::cerr << "Error reading config file - unknown variable name \"" << varName << "\"!\n";

				//we have all our variables - create the system object
				if(!sysName.empty() && !sysPath.empty() &&!sysExtension.empty() && !sysCommand.empty())
				{
					returnVec.push_back(new SystemData(sysName, sysPath, sysExtension, sysCommand));

					//reset the variables for the next block (should there be one)
					sysName = ""; sysPath = ""; sysExtension = "";
				}
			}else{
				std::cerr << "Error reading config file \"" << path << "\" - no equals sign found on line \"" << line << "\"!\n";
				return returnVec;
			}
		}
	}else{
		std::cerr << "Error - could not load config file \"" << path << "\"!\n";
		return returnVec;
	}

	std::cout << "Finished loading config file - created " << returnVec.size() << " systems.\n";
	return returnVec;
}
