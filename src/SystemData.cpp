#include "SystemData.h"
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
#include "FileSorts.h"

std::vector<SystemData*> SystemData::sSystemVector;

namespace fs = boost::filesystem;

SystemData::SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
	const std::string& command, PlatformIds::PlatformId platformId)
{
	mName = name;
	mFullName = fullName;
	mStartPath = startPath;

	//expand home symbol if the startpath contains ~
	if(mStartPath[0] == '~')
	{
		mStartPath.erase(0, 1);
		mStartPath.insert(0, getHomePath());
	}

	mSearchExtensions = extensions;
	mLaunchCommand = command;
	mPlatformId = platformId;

	mRootFolder = new FileData(FOLDER, mStartPath, this);
	mRootFolder->metadata.set("name", mFullName);

	if(!Settings::getInstance()->getBool("PARSEGAMELISTONLY"))
		populateFolder(mRootFolder);

	if(!Settings::getInstance()->getBool("IGNOREGAMELIST"))
		parseGamelist(this);

	mRootFolder->sort(FileSorts::SortTypes.at(0));

	loadTheme();
}

SystemData::~SystemData()
{
	//save changed game data back to xml
	if(!Settings::getInstance()->getBool("IGNOREGAMELIST"))
	{
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
	int timesPlayed = game->metadata.getInt("playcount") + 1;
	game->metadata.set("playcount", std::to_string(static_cast<long long>(timesPlayed)));

	//update last played time
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	game->metadata.setTime("lastplayed", time);
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

	if(!systemList)
	{
		LOG(LogError) << "es_systems.cfg is missing the <systemList> tag!";
		return false;
	}

	for(pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		std::string name, fullname, path, cmd;
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

		SystemData* newSys = new SystemData(name, fullname, path, extensions, cmd, platformId);
		if(newSys->getRootFolder()->getChildren().size() == 0)
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

std::string SystemData::getGamelistPath() const
{
	fs::path filePath;

	filePath = mRootFolder->getPath() / "gamelist.xml";
	if(fs::exists(filePath))
		return filePath.generic_string();

	filePath = getHomePath() + "/.emulationstation/"+ getName() + "/gamelist.xml";
	return filePath.generic_string();
}

std::string SystemData::getThemePath() const
{
	fs::path filePath;

	filePath = mRootFolder->getPath() / "theme.xml";
	if(fs::exists(filePath))
		return filePath.generic_string();

	filePath = getHomePath() + "/.emulationstation/" + getName() + "/theme.xml";
	return filePath.generic_string();
}

bool SystemData::hasGamelist() const
{
	return (fs::exists(getGamelistPath()));
}

unsigned int SystemData::getGameCount() const
{
	return mRootFolder->getFilesRecursive(GAME).size();
}

void SystemData::loadTheme()
{
	mTheme = std::make_shared<ThemeData>();
	try
	{
		mTheme->loadFile(getThemePath());
	} catch(ThemeException& e)
	{
		LOG(LogError) << e.what();
		mTheme = std::make_shared<ThemeData>(); // reset to empty
	}
}
