#include "SystemData.h"
#include "GameData.h"
#include "XMLReader.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>
#include <SDL_joystick.h>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"
#include "Log.h"
#include "InputManager.h"
#include <iostream>
#include "Settings.h"

std::vector<SystemData*> SystemData::sSystemVector;

namespace fs = boost::filesystem;

std::string SystemData::getStartPath() { return mStartPath; }
std::string SystemData::getExtension() { return mSearchExtension; }

SystemData::SystemData(std::string name, std::string descName, std::string startPath, std::string extension, std::string command)
{
	mName = name;
	mDescName = descName;

	//expand home symbol if the startpath contains ~
	if(startPath[0] == '~')
	{
		startPath.erase(0, 1);
		std::string home = getHomePath();
		startPath.insert(0, home);
        }

	mStartPath = startPath;
	mSearchExtension = extension;
	mLaunchCommand = command;

	mRootFolder = new FolderData(this, mStartPath, "Search Root");

	if(!Settings::getInstance()->getBool("PARSEGAMELISTONLY"))
		populateFolder(mRootFolder);

	if(!Settings::getInstance()->getBool("IGNOREGAMELIST"))
		parseGamelist(this);

	mRootFolder->sort();
}

SystemData::~SystemData()
{
	//save changed game data back to xml
	if(!Settings::getInstance()->getBool("IGNOREGAMELIST")) {
		updateGamelist(this);
	}
	delete mRootFolder;
}

std::string strreplace(std::string& str, std::string replace, std::string with)
{
	size_t pos = str.find(replace);

	if(pos != std::string::npos)
		return str.replace(pos, replace.length(), with.c_str(), with.length());
	else
		return str;
}

void SystemData::launchGame(Window* window, GameData* game)
{
	LOG(LogInfo) << "Attempting to launch game...";

	AudioManager::getInstance()->deinit();
	VolumeControl::getInstance()->deinit();
	window->deinit();

	std::string command = mLaunchCommand;

	command = strreplace(command, "%ROM%", game->getBashPath());
	command = strreplace(command, "%BASENAME%", game->getBaseName());
	command = strreplace(command, "%ROM_RAW%", game->getPath());

	LOG(LogInfo) << "	" << command;
	std::cout << "==============================================\n";
	int exitCode = system(command.c_str());
	std::cout << "==============================================\n";

	if(exitCode != 0)
	{
		LOG(LogWarning) << "...launch terminated with nonzero exit code " << exitCode << "!";
	}

	window->init();
	VolumeControl::getInstance()->init();
	AudioManager::getInstance()->init();
	window->normalizeNextUpdate();

	//update number of times the game has been launched and the time
	game->setTimesPlayed(game->getTimesPlayed() + 1);
	game->setLastPlayed(std::time(nullptr));
}

void SystemData::populateFolder(FolderData* folder)
{
	std::string folderPath = folder->getPath();
	if(!fs::is_directory(folderPath))
	{
		LOG(LogWarning) << "Error - folder with path \"" << folderPath << "\" is not a directory!";
		return;
	}

	//make sure that this isn't a symlink to a thing we already have
	if(fs::is_symlink(folderPath))
	{
		//if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if(folderPath.find(fs::canonical(folderPath).string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << folderPath << "\"";
			return;
		}
	}

	for(fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		fs::path filePath = (*dir).path();

		if(filePath.stem().string().empty())
			continue;

		//this is a little complicated because we allow a list of extensions to be defined (delimited with a space)
		//we first get the extension of the file itself:
		std::string extension = filePath.extension().string();
		std::string chkExt;
		size_t extPos = 0;

		//folders *can* also match the extension and be added as games - this is mostly just to support higan
		//see issue #75: https://github.com/Aloshi/EmulationStation/issues/75
		bool isGame = false;
		do {
			//now we loop through every extension in the list
			size_t cpos = extPos;
			extPos = mSearchExtension.find(" ", extPos);
			chkExt = mSearchExtension.substr(cpos, ((extPos == std::string::npos) ? mSearchExtension.length() - cpos: extPos - cpos));

			//if it matches, add it
			if(chkExt == extension)
			{
				GameData* newGame = new GameData(this, filePath.generic_string(), filePath.stem().string());
				folder->pushFileData(newGame);
				isGame = true;
				break;
			}else if(extPos != std::string::npos) //if not, add one to the "next position" marker to skip the space when reading the next extension
			{
				extPos++;
			}

		} while(extPos != std::string::npos && chkExt != "" && chkExt.find(".") != std::string::npos);
	
		//add directories that also do not match an extension as folders
		if(!isGame && fs::is_directory(filePath))
		{
			FolderData* newFolder = new FolderData(this, filePath.generic_string(), filePath.stem().string());
			populateFolder(newFolder);

			//ignore folders that do not contain games
			if(newFolder->getFileCount() == 0)
				delete newFolder;
			else
				folder->pushFileData(newFolder);
		}
	}
}

std::string SystemData::getName()
{
	return mName;
}

std::string SystemData::getDescName()
{
	return mDescName;
}

//creates systems from information located in a config file
bool SystemData::loadConfig(const std::string& path, bool writeExample)
{
	deleteSystems();

	LOG(LogInfo) << "Loading system config file...";

	if(!fs::exists(path))
	{
		LOG(LogInfo) << "System config file \"" << path << "\" doesn't exist!";
		if(writeExample)
			writeExampleConfig(path);

		return false;
	}

	std::ifstream file(path.c_str());
	if(file.is_open())
	{
		size_t lineNr = 0;
		std::string line;
		std::string sysName, sysDescName, sysPath, sysExtension, sysCommand;
		while(file.good())
		{
			lineNr++;
			std::getline(file, line);

			//remove whitespace from line through STL and lambda magic
			line.erase(std::remove_if(line.begin(), line.end(), [&](char c){ return std::string("\t\r\n\v\f").find(c) != std::string::npos; }), line.end());

			//skip blank lines and comments
			if(line.empty() || line.at(0) == '#')
				continue;

			//find the name (left of the equals sign) and the value (right of the equals sign)
			bool lineValid = false;
			std::string varName;
			std::string varValue;
			const std::string::size_type equalsPos = line.find('=', 1);
			if(equalsPos != std::string::npos)
			{
				lineValid = true;
				varName = line.substr(0, equalsPos);
				varValue = line.substr(equalsPos + 1, line.length() - 1);
			}

			if(lineValid)
			{
				//map the value to the appropriate variable
				if(varName == "NAME")
					sysName = varValue;
				else if(varName == "DESCNAME")
					sysDescName = varValue;
				else if(varName == "PATH")
				{
					if(varValue[varValue.length() - 1] == '/')
						sysPath = varValue.substr(0, varValue.length() - 1);
					else
						sysPath = varValue;
					//convert path to generic directory seperators
					boost::filesystem::path genericPath(sysPath);
					sysPath = genericPath.generic_string();
				}
				else if(varName == "EXTENSION")
					sysExtension = varValue;
				else if(varName == "COMMAND")
					sysCommand = varValue;

				//we have all our variables - create the system object
				if(!sysName.empty() && !sysPath.empty() &&!sysExtension.empty() && !sysCommand.empty())
				{
					if(sysDescName.empty())
						sysDescName = sysName;

					SystemData* newSystem = new SystemData(sysName, sysDescName, sysPath, sysExtension, sysCommand);
					if(newSystem->getRootFolder()->getFileCount() == 0)
					{
						LOG(LogWarning) << "System \"" << sysName << "\" has no games! Ignoring it.";
						delete newSystem;
					}else{
						sSystemVector.push_back(newSystem);
					}

					//reset the variables for the next block (should there be one)
					sysName = ""; sysDescName = ""; sysPath = ""; sysExtension = ""; sysCommand = "" ;
				}
			}else{
				LOG(LogError) << "Error reading config file \"" << path << "\" - no equals sign found on line " << lineNr << ": \"" << line << "\"!";
				return false;
			}
		}
	}else{
		LOG(LogError) << "Error - could not load config file \"" << path << "\"!";
		return false;
	}

	LOG(LogInfo) << "Finished loading config file - created " << sSystemVector.size() << " systems.";
	return true;
}

void SystemData::writeExampleConfig(const std::string& path)
{
	std::cerr << "Writing example config to \"" << path << "\"...";

	std::ofstream file(path.c_str());

	file << "# This is the EmulationStation Systems configuration file." << std::endl;
	file << "# Lines that begin with a hash (#) are ignored, as are empty lines." << std::endl;
	file << "# A sample system might look like this:" << std::endl;
	file << "#NAME=nes" << std::endl;
	file << "#DESCNAME=Nintendo Entertainment System" << std::endl;
	file << "#PATH=~/ROMs/nes/" << std::endl;
	file << "#EXTENSION=.nes .NES" << std::endl;
	file << "#COMMAND=retroarch -L ~/cores/libretro-fceumm.so %ROM%" << std::endl << std::endl;

	file << "#NAME is a short name used internally (and in alternative paths)." << std::endl;
	file << "#DESCNAME is a descriptive name to identify the system. It may be displayed in a header." << std::endl;
	file << "#PATH is the path to start the recursive search for ROMs in. ~ will be expanded into the $HOME variable." << std::endl;
	file << "#EXTENSION is a list of extensions to search for, separated by spaces. You MUST include the period, and it must be exact - it's case sensitive, and no wildcards." << std::endl;
	file << "#COMMAND is the shell command to execute when a game is selected. %ROM% will be replaced with the (bash special-character escaped) path to the ROM." << std::endl << std::endl;

	file << "#Now try your own!" << std::endl;
	file << "NAME=" << std::endl;
	file << "DESCNAME=" << std::endl;
	file << "PATH=" << std::endl;
	file << "EXTENSION=" << std::endl;
	file << "COMMAND=" << std::endl;

	file.close();

	std::cerr << "done. Go read it!\n";
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
	std::string home = getHomePath();
	if(home.empty())
	{
		LOG(LogError) << "$HOME environment variable empty or nonexistant!";
		exit(1);
		return "";
	}

	return(home + "/.emulationstation/es_systems.cfg");
}

FolderData* SystemData::getRootFolder()
{
	return mRootFolder;
}

std::string SystemData::getGamelistPath()
{
	std::string filePath;

	filePath = mRootFolder->getPath() + "/gamelist.xml";
	if(fs::exists(filePath))
		return filePath;

	filePath = getHomePath();
	filePath += "/.emulationstation/"+ getName() + "/gamelist.xml";
	if(fs::exists(filePath))
		return filePath;

	return "";
}

bool SystemData::hasGamelist()
{
	if(getGamelistPath().empty())
		return false;
	else
		return true;
}
