#include "SystemData.h"
#include "GameData.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>
#include <SDL/SDL_joystick.h>

std::vector<SystemData*> SystemData::sSystemVector;

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

	//suspend SDL joystick events (these'll pile up even while something else is running)
	SDL_JoystickEventState(0);

	std::string command = mLaunchCommand;
	GameData* game = mGameVector.at(i);

	command = strreplace(command, "%ROM%", game->getValidPath());

	std::cout << "	" << command << "\n";
	std::cout << "=====================================================\n";
	system(command.c_str());
	std::cout << "=====================================================\n";

	std::cout << "...launch terminated!\n";

	//re-enable SDL joystick events
	SDL_JoystickEventState(1);
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

	//expand home symbol if necessary
	if(mStartPath[0] == '~')
	{
		mStartPath.erase(0, 1);

		std::string home = getenv("HOME");
		if(home.empty())
		{
			std::cerr << "ERROR - System start path contains ~ but $HOME is not set!\n";
			return;
		}else{
			mStartPath.insert(0, home);
		}
	}

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




//creates systems from information located in a config file
void SystemData::loadConfig()
{
	deleteSystems();

	std::string path = getConfigPath();

	std::cout << "Loading system config file \"" << path << "\"...\n";

	std::ifstream file(path.c_str());
	if(file.is_open())
	{
		std::string line;
		std::string sysName, sysPath, sysExtension, sysCommand;
		while(file.good())
		{
			std::getline(file, line);

			//skip blank lines and comments
			if(line.empty() || line[0] == *"#")
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
				//map the value to the appropriate variable
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
					sSystemVector.push_back(new SystemData(sysName, sysPath, sysExtension, sysCommand));

					//reset the variables for the next block (should there be one)
					sysName = ""; sysPath = ""; sysExtension = ""; sysCommand = "";
				}
			}else{
				std::cerr << "Error reading config file \"" << path << "\" - no equals sign found on line \"" << line << "\"!\n";
				return;
			}
		}
	}else{
		std::cerr << "Error - could not load config file \"" << path << "\"!\n";
		return;
	}

	std::cout << "Finished loading config file - created " << sSystemVector.size() << " systems.\n";
	return;
}

void SystemData::writeExampleConfig()
{
	std::string path = getConfigPath();

	std::ofstream file(path.c_str());

	file << "# This is the EmulationStation Systems configuration file." << std::endl;
	file << "# Lines that begin with a hash (#) are ignored, as are empty lines." << std::endl;
	file << "# A sample system might look like this:" << std::endl;
	file << "#NAME=Nintendo Entertainment System" << std::endl;
	file << "#PATH=~/ROMs/nes/" << std::endl;
	file << "#EXTENSION=.nes" << std::endl;
	file << "#COMMAND=retroarch -L ~/cores/libretro-fceumm.so %ROM%" << std::endl << std::endl;

	file << "#NAME is just a name to identify the system." << std::endl;
	file << "#PATH is the path to start the recursive search for ROMs in. ~ will be expanded into the $HOME variable." << std::endl;
	file << "#EXTENSION is the exact extension to search for. You MUST include the period, and it must be exact - no regex or wildcard support (sorry!)." << std::endl;
	file << "#COMMAND is the shell command to execute when a game is selected. %ROM% will be replaced with the path to the ROM." << std::endl << std::endl;

	file << "#Now try your own!" << std::endl;
	file << "NAME=" << std::endl;
	file << "PATH=" << std::endl;
	file << "EXTENSION=" << std::endl;
	file << "COMMAND=" << std::endl;

	file.close();
}

void SystemData::deleteSystems()
{
	for(unsigned int i = 0; i < sSystemVector.size(); i++)
	{
		delete sSystemVector.at(i);
	}
	sSystemVector.clear();
}

std::string SystemData::getConfigPath()
{
	std::string home = getenv("HOME");
	if(home.empty())
	{
		std::cerr << "FATAL ERROR - $HOME environment variable empty or nonexistant!\n";
		exit(1);
		return "";
	}

	return(home + "/.es_systems.cfg");
}
