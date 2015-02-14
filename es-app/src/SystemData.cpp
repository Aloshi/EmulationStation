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

std::string SystemData::replaceOptions( std::string command, FileData game ) const
{
	// generate system options for this game by building up
	// one string per 'section', and then replace that section in the command

	std::map<std::string, std::stringstream> sections;

	std::vector<FileData> fileStack = game.getParents();
	fileStack.insert( fileStack.begin(), game );
	std::map<std::string, std::string> optionvalues = SystemOption::getInheritedOptions( fileStack, FileOptionType::SYSTEM );

	for( auto option = mOptions.begin(); option != mOptions.end(); option++ )
	{
		std::string id = (*option)->getID();
		std::string section = (*option)->getReplace();
		std::string valueID = optionvalues.count(id) > 0 ? optionvalues[id] : (*option)->getDefaultID();
		sections[ section ] << "";

		if( !valueID.empty() )
		{
			SystemOptionValue* value = (*option)->getValue( valueID );

			if( value )
			{
				if( sections[ section ].tellp() > 0 )
					sections[ section ] << " ";

				sections[ section ] << value->getCode();
			}
		}
	}

	// now we have a list of sections and their replacement text,
	// go thru all of them and replace the text in the command
	for( auto sectionAdder = sections.begin(); sectionAdder != sections.end(); sectionAdder++ )
		command = strreplace( command, sectionAdder->first, sectionAdder->second.str() );
	
	return command;
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
	const std::string rom_raw = game.getPath().string();

	command = strreplace(command, "%ROM%", rom);
	command = strreplace(command, "%BASENAME%", basename);
	command = strreplace(command, "%ROM_RAW%", rom_raw);

	command = replaceOptions( command, game );

	LOG(LogInfo) << "	" << command;
	std::cout << "==============================================\n";
	std::cout << command << "\n";
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

	// update number of times the game has been launched
	MetaDataMap metadata = game.get_metadata();
	int timesPlayed = metadata.get<int>("playcount") + 1;
	metadata.set("playcount", timesPlayed);

	// update last played time
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	metadata.set("lastplayed", time);

	game.set_metadata(metadata);
}

std::string SystemData::getGamelistPath(bool forWrite) const
{
	fs::path filePath;

	filePath = mStartPath + "/gamelist.xml";
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
