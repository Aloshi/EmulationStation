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
std::vector<std::string> SystemData::getExtensions() { return mSearchExtensions; }
std::string SystemData::getScreenshotDir() { return mScreenshotDir; }
std::string SystemData::getEmulatorScreenshotDumpDir() { return mEmulatorScreenshotDumpDir; }

namespace
{
	//expand home symbol if the startpath contains ~
	void expandTilde(std::string &path)
	{
		if(path[0] == '~')
		{
			path.erase(0, 1);
			path.insert(0, getHomePath());
		}
	}
}

SystemData::SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
	const std::string& command, const std::string &emulatorScreenshotDumpDir, const std::string &screenshotDir, PlatformIds::PlatformId platformId)
{
	mName = name;
	mFullName = fullName;
	mStartPath = startPath;
	mScreenshotDir = screenshotDir;
	mEmulatorScreenshotDumpDir = emulatorScreenshotDumpDir;

	expandTilde(mStartPath);
	expandTilde(mScreenshotDir);
	expandTilde(mEmulatorScreenshotDumpDir);

	mSearchExtensions = extensions;
	mLaunchCommand = command;
	mPlatformId = platformId;

	mRootFolder = new FolderData(this, mStartPath, "Search Root");

	if(!Settings::getInstance()->getBool("PARSEGAMELISTONLY"))
		populateFolder(mRootFolder);

	if(!Settings::getInstance()->getBool("IGNOREGAMELIST"))
		parseGamelist(this);

	mRootFolder->sort();
        mRootFolder->reselect();
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
	game->incTimesPlayed();
	game->lastPlayedNow();
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

	fs::path filePath;
	std::string extension;
	bool isGame;
	for(fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		filePath = (*dir).path();

		if(filePath.stem().string().empty())
			continue;

		//this is a little complicated because we allow a list of extensions to be defined (delimited with a space)
		//we first get the extension of the file itself:
		extension = filePath.extension().string();
		
		//fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
		//see issue #75: https://github.com/Aloshi/EmulationStation/issues/75

		isGame = false;
		if(std::find(mSearchExtensions.begin(), mSearchExtensions.end(), extension) != mSearchExtensions.end())
		{
			GameData* newGame = new GameData(filePath.generic_string(), MetaDataList(getGameMDD()));
			folder->pushFileData(newGame);
			isGame = true;
		}

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

std::string SystemData::getFullName()
{
	if(mFullName.empty())
		return mName;
	else
		return mFullName;
}

//creates systems from information located in a config file
bool SystemData::loadConfig(const std::string& path, bool writeExample)
{
	deleteSystems();

	LOG(LogInfo) << "Loading system config file " << path << "...";

	if(!fs::exists(path))
	{
		LOG(LogError) << "File does not exist!";
		
		if(writeExample)
			writeExampleConfig(path);

		return false;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());

	if(!res)
	{
		LOG(LogError) << "Could not parse config file!";
		LOG(LogError) << res.description();
		return false;
	}

	//actually read the file
	pugi::xml_node systemList = doc.child("systemList");

	for(pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		std::string name, fullname, path, cmd, emulatorScreenshotDumpDir, screenshotDir;
		PlatformIds::PlatformId platformId = PlatformIds::PLATFORM_UNKNOWN;

		name = system.child("name").text().get();
		fullname = system.child("fullname").text().get();
		path = system.child("path").text().get();

		//convert extensions list from a string into a vector of strings
		const pugi::char_t* extStr = system.child("extension").text().get();
		std::vector<std::string> extensions;
		std::vector<char> buff(strlen(extStr) + 1);
		strcpy(buff.data(), extStr);
		char* ext = strtok(buff.data(), " ");
		while(ext != NULL)
		{
			extensions.push_back(ext);
			ext = strtok(NULL, " ");
		}

		cmd = system.child("command").text().get();
                emulatorScreenshotDumpDir = system.child("emulatorScreenshotDumpDir").text().get();
                screenshotDir = system.child("screenshotDir").text().get();
		platformId = (PlatformIds::PlatformId)system.child("platformid").text().as_uint(PlatformIds::PLATFORM_UNKNOWN);

		//validate
		if(name.empty() || path.empty() || extensions.empty() || cmd.empty())
		{
			LOG(LogError) << "System \"" << name << "\" is missing name, path, extension, or command!";
			continue;
		}

		//convert path to generic directory seperators
		boost::filesystem::path genericPath(path);
		path = genericPath.generic_string();

		SystemData* newSys = new SystemData(name, fullname, path, extensions, cmd, emulatorScreenshotDumpDir, screenshotDir, platformId);
		if(newSys->getRootFolder()->getFileCount() == 0)
		{
			LOG(LogWarning) << "System \"" << name << "\" has no games! Ignoring it.";
			delete newSys;
		}else{
			sSystemVector.push_back(newSys);
		}
	}

	return true;
}

void SystemData::writeExampleConfig(const std::string& path)
{
	std::ofstream file(path.c_str());

	file << "<!-- This is the EmulationStation Systems configuration file.\n"
			"All systems must be contained within the <systemList> tag.-->\n"
			"\n"
			"<systemList>\n"
			"	<!-- Here's an example system to get you started. -->\n"
			"	<system>\n"
			"\n"
			"		<!-- A short name, used internally. Traditionally lower-case. -->\n"
			"		<name>nes</name>\n"
			"\n"
			"		<!-- A \"pretty\" name, displayed in the header and such. -->\n"
			"		<fullname>Nintendo Entertainment System</fullname>\n"
			"\n"
			"		<!-- The path to start searching for ROMs in. '~' will be expanded to $HOME or $HOMEPATH, depending on platform. -->\n"
			"		<path>~/roms/nes</path>\n"
			"\n"
			"		<!-- A list of extensions to search for, delimited by a space. You MUST include the period! It's also case sensitive. -->\n"
			"		<extension>.nes .NES</extension>\n"
			"\n"
			"		<!-- The shell command executed when a game is selected. A few special tags are replaced if found in a command:\n"
			"		%ROM% is replaced by a bash-special-character-escaped absolute path to the ROM.\n"
			"		%BASENAME% is replaced by the \"base\" name of the ROM.  For example, \"/foo/bar.rom\" would have a basename of \"bar\". Useful for MAME.\n"
			"		%ROM_RAW% is the raw, unescaped path to the ROM. -->\n"
			"		<command>retroarch -L ~/cores/libretro-fceumm.so %ROM%</command>\n"
			"\n"
			"		<!-- Where does the emulator store screenshots? Screenshots created during playing a game will be automatically moved to the screenshotDir and added to the game. -->\n"
			"		<emulatorScreenshotDumpDir>~/retroarch/screenshots</emulatorScreenshotDumpDir>\n"
			"		<!-- New screenshots will be renamed to %ROMNAME%-<id>.png and stored in the screenshotDir -->\n"
			"		<screenshotDir>~/snaps/nes/</screenshotDir>\n"
			"\n"
			"	</system>\n"
			"</systemList>\n";

	file.close();

	LOG(LogError) << "Example config written!  Go read it at \"" << path << "\"!";
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
		LOG(LogError) << "Home path environment variable empty or nonexistant!";
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
	return filePath;
}

bool SystemData::hasGamelist()
{
	return (fs::exists(getGamelistPath()));
}

std::vector<MetaDataDecl> SystemData::getGameMDD()
{
	return MetaDataList::getDefaultGameMDD();
}

PlatformIds::PlatformId SystemData::getPlatformId()
{
	return mPlatformId;
}

unsigned int SystemData::getGameCount()
{
	return mRootFolder->getFilesRecursive(true).size();
}
