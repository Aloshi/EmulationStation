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

namespace fs = boost::filesystem;

SystemData::SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
	const std::string& command, const std::vector<PlatformIds::PlatformId>& platformIds, const std::string& themeFolder) :
	mRoot(FileData(".", this, FileType::FOLDER))
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

	loadTheme();
}

SystemData::~SystemData()
{
}


std::string strreplace(std::string str, const std::string& replace, const std::string& with)
{
	size_t pos;
	while((pos = str.find(replace)) != std::string::npos)
		str = str.replace(pos, replace.length(), with.c_str(), with.length());
	
	return str;
}

// plaform-specific escape path function
// on windows: just puts the path in quotes
// everything else: assume bash and escape special characters with backslashes
std::string escapePath(const boost::filesystem::path& path)
{
#ifdef WIN32
	// windows escapes stuff by just putting everything in quotes
	return '"' + fs::path(path).make_preferred().string() + '"';
#else
	// a quick and dirty way to insert a backslash before most characters that would mess up a bash path
	std::string pathStr = path.string();

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
#endif
}

void SystemData::launchGame(Window* window, FileData game) const
{
	LOG(LogInfo) << "Attempting to launch game...";

	AudioManager::getInstance()->deinit();
	VolumeControl::getInstance()->deinit();
	window->deinit();

	std::string command = mLaunchCommand;

	const std::string rom = escapePath(game.getPath());
	const std::string basename = game.getPath().stem().string();
	const std::string rom_raw = fs::path(game.getPath()).make_preferred().string();

	command = strreplace(command, "%ROM%", rom);
	command = strreplace(command, "%BASENAME%", basename);
	command = strreplace(command, "%ROM_RAW%", rom_raw);

	LOG(LogInfo) << "	" << command;
	std::cout << "==============================================\n";
	int exitCode = runSystemCommand(command);
	std::cout << "==============================================\n";

	if(exitCode != 0)
	{
		LOG(LogWarning) << "...launch terminated with nonzero exit code " << exitCode << "!";
	}

	window->init();
	VolumeControl::getInstance()->init();
	AudioManager::getInstance()->init();
	window->normalizeNextUpdate();

	// update number of times the game has been launched
	MetaDataMap metadata = game.get_metadata();
	int timesPlayed = metadata.get<int>("playcount") + 1;
	metadata.set("playcount", timesPlayed);

	// update last played time
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	metadata.set("lastplayed", time);

	game.set_metadata(metadata);
}

std::string SystemData::getThemePath() const
{
	// where we check for themes, in order:
	// 1. [SYSTEM_PATH]/theme.xml
	// 2. currently selected theme set

	// first, check game folder
	fs::path localThemePath = mStartPath + "/theme.xml";
	if(fs::exists(localThemePath))
		return localThemePath.generic_string();

	// not in game folder, try theme sets
	return ThemeData::getThemeFromCurrentSet(mThemeFolder).generic_string();
}

unsigned int SystemData::getGameCount() const
{
	return getRootFolder().getChildrenRecursive(false).size();
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

bool SystemData::hasFileWithImage() const
{
	// TODO: optimize this with an SQL query
	std::vector<FileData> files = getRootFolder().getChildrenRecursive(true);
	for(auto it = files.begin(); it != files.end(); it++)
	{
		if(!it->get_metadata().get<std::string>("image").empty())
			return true;
	}

	return false;
}
