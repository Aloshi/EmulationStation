#include "SystemData.h"
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
#include "FileSorts.h"

namespace fs = boost::filesystem;

SystemData::SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
	const std::string& command, const std::vector<PlatformIds::PlatformId>& platformIds, const std::string& themeFolder)
{
	mName = name;
	mFullName = fullName;
	mStartPath = startPath;

	// expand home symbol if the startpath contains ~
	if(mStartPath[0] == '~')
	{
		mStartPath.erase(0, 1);
		mStartPath.insert(0, getHomePath());
	}

	mSearchExtensions = extensions;
	mLaunchCommand = command;
	mPlatformIds = platformIds;
	mThemeFolder = themeFolder;

	mRootFolder = new FileData(FOLDER, mStartPath, this);
	mRootFolder->metadata.set("name", mFullName);

	if(!Settings::getInstance()->getBool("ParseGamelistOnly"))
		populateFolder(mRootFolder);

	mRootFolder->sort(FileSorts::SortTypes.at(0));

	loadTheme();
}

SystemData::~SystemData()
{
	delete mRootFolder;
}

std::string strreplace(std::string str, const std::string& replace, const std::string& with)
{
	size_t pos;
	while((pos = str.find(replace)) != std::string::npos)
		str = str.replace(pos, replace.length(), with.c_str(), with.length());
	
	return str;
}

std::string escapePath(const boost::filesystem::path& path)
{
	// a quick and dirty way to insert a backslash before most characters that would mess up a bash path;
	// someone with regex knowledge should make this into a one-liner
	std::string pathStr = path.generic_string();

	const char* invalidChars = " '\"\\!$^&*(){}[]?;<>";
	for(unsigned int i = 0; i < pathStr.length(); i++)
	{
		char c;
		unsigned int charNum = 0;
		do {
			c = invalidChars[charNum];
			if(pathStr[i] == c)
			{
				pathStr.insert(i, "\\");
				i++;
				break;
			}
			charNum++;
		} while(c != '\0');
	}

	return pathStr;
}

void SystemData::launchGame(Window* window, FileData* game)
{
	LOG(LogInfo) << "Attempting to launch game...";

	AudioManager::getInstance()->deinit();
	VolumeControl::getInstance()->deinit();
	window->deinit();

	std::string command = mLaunchCommand;

	const std::string rom = escapePath(game->getPath());
	const std::string basename = game->getPath().stem().string();
	const std::string rom_raw = game->getPath().string();

	command = strreplace(command, "%ROM%", rom);
	command = strreplace(command, "%BASENAME%", basename);
	command = strreplace(command, "%ROM_RAW%", rom_raw);

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

	//update number of times the game has been launched
	int timesPlayed = game->metadata.get<int>("playcount") + 1;
	game->metadata.set("playcount", timesPlayed);

	//update last played time
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	game->metadata.set("lastplayed", time);
}

void SystemData::populateFolder(FileData* folder)
{
	const fs::path& folderPath = folder->getPath();
	if(!fs::is_directory(folderPath))
	{
		LOG(LogWarning) << "Error - folder with path \"" << folderPath << "\" is not a directory!";
		return;
	}

	const std::string folderStr = folderPath.generic_string();

	//make sure that this isn't a symlink to a thing we already have
	if(fs::is_symlink(folderPath))
	{
		//if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if(folderStr.find(fs::canonical(folderPath).generic_string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << folderPath << "\"";
			return;
		}
	}

	fs::path filePath;
	std::string extension;
	bool isGame;
	for(fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		filePath = (*dir).path();

		if(filePath.stem().empty())
			continue;

		//this is a little complicated because we allow a list of extensions to be defined (delimited with a space)
		//we first get the extension of the file itself:
		extension = filePath.extension().string();
		
		//fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
		//see issue #75: https://github.com/Aloshi/EmulationStation/issues/75

		isGame = false;
		if(std::find(mSearchExtensions.begin(), mSearchExtensions.end(), extension) != mSearchExtensions.end())
		{
			FileData* newGame = new FileData(GAME, filePath.generic_string(), this);
			folder->addChild(newGame);
			isGame = true;
		}

		//add directories that also do not match an extension as folders
		if(!isGame && fs::is_directory(filePath))
		{
			FileData* newFolder = new FileData(FOLDER, filePath.generic_string(), this);
			populateFolder(newFolder);

			//ignore folders that do not contain games
			if(newFolder->getChildren().size() == 0)
				delete newFolder;
			else
				folder->addChild(newFolder);
		}
	}
}

std::string SystemData::getGamelistPath(bool forWrite) const
{
	fs::path filePath;

	filePath = mRootFolder->getPath() / "gamelist.xml";
	if(fs::exists(filePath))
		return filePath.generic_string();

	filePath = getHomePath() + "/.emulationstation/gamelists/" + mName + "/gamelist.xml";
	if(forWrite) // make sure the directory exists if we're going to write to it, or crashes will happen
		fs::create_directories(filePath.parent_path());
	if(forWrite || fs::exists(filePath))
		return filePath.generic_string();

	return "/etc/emulationstation/gamelists/" + mName + "/gamelist.xml";
}

std::string SystemData::getThemePath() const
{
	// where we check for themes, in order:
	// 1. [SYSTEM_PATH]/theme.xml
	// 2. currently selected theme set

	// first, check game folder
	fs::path localThemePath = mRootFolder->getPath() / "theme.xml";
	if(fs::exists(localThemePath))
		return localThemePath.generic_string();

	// not in game folder, try theme sets
	return ThemeData::getThemeFromCurrentSet(mThemeFolder).generic_string();
}

unsigned int SystemData::getGameCount() const
{
	return mRootFolder->getFilesRecursive(GAME).size();
}

void SystemData::loadTheme()
{
	mTheme = std::make_shared<ThemeData>();

	std::string path = getThemePath();

	if(!fs::exists(path)) // no theme available for this platform
		return;

	try
	{
		mTheme->loadFile(path);
	} catch(ThemeException& e)
	{
		LOG(LogError) << e.what();
		mTheme = std::make_shared<ThemeData>(); // reset to empty
	}
}
