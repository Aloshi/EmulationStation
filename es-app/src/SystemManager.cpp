#include "SystemManager.h"
#include "Log.h"
#include "ESException.h"
#include "SystemData.h"
#include <fstream>
#include <pugixml/pugixml.hpp>
#include "Settings.h"
#include "views/ViewController.h"

namespace fs = boost::filesystem;

SystemManager* SystemManager::sInstance = NULL;

SystemManager* SystemManager::getInstance()
{
	if(!sInstance)
		sInstance = new SystemManager();

	return sInstance;
}

std::string SystemManager::getDatabasePath()
{
	return getHomePath() + "/.emulationstation/gamelist.db";
}

SystemManager::SystemManager() : mDatabase(getDatabasePath())
{
}

SystemManager::~SystemManager()
{
	// delete all systems
	for(auto it = mSystems.begin(); it != mSystems.end(); it++)
		delete *it;
}

// helper function for reading lists into vectors
std::vector<std::string> readList(const std::string& str, const char* delims = " \t\r\n,")
{
	std::vector<std::string> ret;

	size_t prevOff = str.find_first_not_of(delims, 0);
	size_t off = str.find_first_of(delims, prevOff);
	while(off != std::string::npos || prevOff != std::string::npos)
	{
		ret.push_back(str.substr(prevOff, off - prevOff));

		prevOff = str.find_first_not_of(delims, off);
		off = str.find_first_of(delims, prevOff);
	}

	return ret;
}

void SystemManager::loadConfig()
{
	// delete systems (if any already exist)
	for(auto it = mSystems.begin(); it != mSystems.end(); it++)
		delete *it;
	mSystems.clear();

	const fs::path configPath = getConfigPath(false);

	LOG(LogInfo) << "Loading system config file " << configPath << "...";

	if(!fs::exists(configPath))
	{
		writeExampleConfig(getConfigPath(true));
		throw ESException() << "The es_systems.cfg file does not exist!\nAn example has been written at\n" << getConfigPath(true);
	}

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(configPath.c_str());

	// pugixml encountered malformed XML
	if(!res)
		throw ESException() << "Could not parse es_systems.cfg file!\n" << res.description();

	pugi::xml_node systemList = doc.child("systemList");
	if(!systemList)
		throw ESException() << "es_systems.cfg is missing the <systemList> tag!";

	// load each system in the config file
	for(pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		std::string name, fullname, path, cmd, themeFolder;
		PlatformIds::PlatformId platformId = PlatformIds::PLATFORM_UNKNOWN;

		name = system.child("name").text().get();
		fullname = system.child("fullname").text().get();
		path = system.child("path").text().get();

		// convert extensions list from a string into a vector of strings
		std::vector<std::string> extensions = readList(system.child("extension").text().get());

		cmd = system.child("command").text().get();

		// platform id list
		const char* platformList = system.child("platform").text().get();
		std::vector<std::string> platformStrs = readList(platformList);
		std::vector<PlatformIds::PlatformId> platformIds;
		for(auto it = platformStrs.begin(); it != platformStrs.end(); it++)
		{
			const char* str = it->c_str();
			PlatformIds::PlatformId platformId = PlatformIds::getPlatformId(str);

			if(platformId == PlatformIds::PLATFORM_IGNORE)
			{
				// when platform is ignore, do not allow other platforms
				platformIds.clear();
				platformIds.push_back(platformId);
				break;
			}

			// if there appears to be an actual platform ID supplied but it didn't match the list, warn
			if(str != NULL && str[0] != '\0' && platformId == PlatformIds::PLATFORM_UNKNOWN)
				LOG(LogWarning) << "  Unknown platform for system \"" << name << "\" (platform \"" << str << "\" from list \"" << platformList << "\")";
			else if(platformId != PlatformIds::PLATFORM_UNKNOWN)
				platformIds.push_back(platformId);
		}

		// theme folder
		themeFolder = system.child("theme").text().as_string(name.c_str());

		// validate as best we can (make sure we're not missing required information)
		if(!isValidSystemName(name) || path.empty() || extensions.empty() || cmd.empty())
			throw ESException() << "System \"" << name << "\" is missing name, path, extension, or command!";

		// convert path to generic directory seperators
		boost::filesystem::path genericPath(path);
		path = genericPath.generic_string();

		// strip trailing slash
		if(path[path.size() - 1] == '/')
			path = path.erase(path.size() - 1);

		SystemData* newSys = new SystemData(name, fullname, path, extensions, cmd, platformIds, themeFolder);
		mDatabase.addMissingFiles(newSys);
		mDatabase.updateExists(newSys);

		if(newSys->getGameCount() == 0)
		{
			LOG(LogWarning) << "System \"" << name << "\" has no games! Ignoring it.";
			delete newSys;
		}else{
			mSystems.push_back(newSys);
		}
	}

	updateDatabase();
}

void SystemManager::writeExampleConfig(const fs::path& path)
{
	std::ofstream file(path.string());

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
		"		<!-- A \"pretty\" name, displayed in menus and such. -->\n"
		"		<fullname>Nintendo Entertainment System</fullname>\n"
		"\n"
		"		<!-- The path to start searching for ROMs in. '~' will be expanded to $HOME on Linux or %HOMEPATH% on Windows. -->\n"
		"		<path>~/roms/nes</path>\n"
		"\n"
		"		<!-- A list of extensions to search for, delimited by any of the whitespace characters (\", \\r\\n\\t\").\n"
		"		You MUST include the period at the start of the extension! It's also case sensitive. -->\n"
		"		<extension>.nes .NES</extension>\n"
		"\n"
		"		<!-- The shell command executed when a game is selected. A few special tags are replaced if found in a command:\n"
		"		%ROM% is replaced by a bash-special-character-escaped absolute path to the ROM.\n"
		"		%BASENAME% is replaced by the \"base\" name of the ROM.  For example, \"/foo/bar.rom\" would have a basename of \"bar\". Useful for MAME.\n"
		"		%ROM_RAW% is the raw, unescaped path to the ROM. -->\n"
		"		<command>retroarch -L ~/cores/libretro-fceumm.so %ROM%</command>\n"
		"\n"
		"		<!-- The platform to use when scraping. You can see the full list of accepted platforms in src/PlatformIds.cpp.\n"
		"		It's case sensitive, but everything is lowercase. This tag is optional.\n"
		"		You can use multiple platforms too, delimited with any of the whitespace characters (\", \\r\\n\\t\"), eg: \"genesis, megadrive\" -->\n"
		"		<platform>nes</platform>\n"
		"\n"
		"		<!-- The theme to load from the current theme set.  See THEMES.md for more information.\n"
		"		This tag is optional. If not set, it will default to the value of <name>. -->\n"
		"		<theme>nes</theme>\n"
		"	</system>\n"
		"</systemList>\n";

	file.close();

	LOG(LogError) << "Example config written!  Go read it at \"" << path << "\"!";
}

fs::path SystemManager::getConfigPath(bool forWrite)
{
	fs::path path = getHomePath() + "/.emulationstation/es_systems.cfg";
	if(forWrite || fs::exists(path))
		return path.generic_string();

	return "/etc/emulationstation/es_systems.cfg";
}

SystemData* SystemManager::getNext(SystemData* system) const
{
	auto it = getIterator(system);
	it++;
	if(it == mSystems.end())
		return *mSystems.begin();
	else 
		return *it;
}

SystemData* SystemManager::getPrev(SystemData* system) const
{
	auto it = getRevIterator(system);
	it++;
	if(it == mSystems.rend())
		return *mSystems.rbegin();
	else
		return *it;
}

bool SystemManager::isValidSystemName(const std::string& name)
{
	if(name.empty())
		return false;

	const std::string invalid_chars = "/\\";
	for(unsigned int i = 0; i < name.size(); i++)
	{
		for(unsigned int j = 0; j < invalid_chars.size(); j++)
			if(name[i] == invalid_chars[j])
				return false;
	}

	return true;
}

SystemData* SystemManager::getSystemByName(const std::string& name) const
{
	for(auto it = mSystems.begin(); it != mSystems.end(); it++)
		if((*it)->getName() == name)
			return *it;

	return NULL;
}

void SystemManager::updateDatabase()
{
	for(auto it = mSystems.begin(); it != mSystems.end(); it++)
	{
		mDatabase.addMissingFiles(*it);
		mDatabase.updateExists(*it);
	}
}

fs::path SystemManager::getGamelistXMLPath(const SystemData* sys, bool forWrite)
{
	fs::path filePath;

	filePath = sys->getStartPath() + "/gamelist.xml";
	if(fs::exists(filePath))
		return filePath;

	filePath = getHomePath() + "/.emulationstation/gamelists/" + sys->getName() + "/gamelist.xml";
	if(forWrite) // make sure the directory exists if we're going to write to it, or crashes will happen
		fs::create_directories(filePath.parent_path());
	if(forWrite || fs::exists(filePath))
		return filePath;

	return "/etc/emulationstation/gamelists/" + sys->getName() + "/gamelist.xml";
}

bool SystemManager::hasNewGamelistXML(const SystemData* sys)
{
	fs::path path = getGamelistXMLPath(sys, false);
	if(!fs::exists(path))
		return false;

	std::time_t time = fs::last_write_time(path);
	if(time == (std::time_t)(-1))
		return false;

	return time > Settings::getInstance()->getTime("LastXMLImportTime");
}

bool SystemManager::hasNewGamelistXML() const
{
	for(auto it = mSystems.begin(); it != mSystems.end(); it++)
	{
		if(hasNewGamelistXML(*it))
			return true;
	}
	return false;
}

void SystemManager::importGamelistXML(bool onlyNew)
{
	for(auto it = mSystems.begin(); it != mSystems.end(); it++)
	{
		if(onlyNew && !hasNewGamelistXML(*it))
			continue;

		boost::filesystem::path path = getGamelistXMLPath(*it, false);
		if(boost::filesystem::exists(path))
			database().importXML(*it, path.generic_string());

		database().updateExists(*it);
		ViewController::get()->onFilesChanged(*it);
	}

	std::time_t now;
	time(&now);
	Settings::getInstance()->setTime("LastXMLImportTime", now);
}
