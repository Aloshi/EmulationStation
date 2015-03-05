#pragma once

#include <boost/filesystem.hpp>
#include <vector>

#include "GamelistDB.h"

class SystemData;

class SystemManager
{
public:
	static SystemManager* getInstance();

	SystemManager();
	virtual ~SystemManager();

	inline const std::vector<SystemData*>& getSystems() const { return mSystems; }

	SystemData* getSystemByName(const std::string& name) const;

	// iterators
	inline std::vector<SystemData*>::const_iterator getIterator(SystemData* system) const { return std::find(mSystems.begin(), mSystems.end(), system); };
	inline std::vector<SystemData*>::const_reverse_iterator getRevIterator(SystemData* system) const { return std::find(mSystems.rbegin(), mSystems.rend(), system); };

	SystemData* getNext(SystemData* system) const;
	SystemData* getPrev(SystemData* system) const;

	// Load the system config file at getConfigPath(false).
	// An exception will be thrown if the file was not successfully read for any reason.
	// An example will be written if the file doesn't exist.
	// Any pre-existing systems (in mSystems) will be deleted!
	void loadConfig();

	inline GamelistDB& database() { return mDatabase; }

	bool hasNewGamelistXML() const;
	void importGamelistXML(bool onlyNew);
	static boost::filesystem::path getGamelistXMLPath(const SystemData* sys, bool forWrite);

private:
	static SystemManager* sInstance;
	
	std::vector<SystemData*> mSystems;
	GamelistDB mDatabase;

	static bool hasNewGamelistXML(const SystemData* sys);

	// if forWrite is true, will only return ~/.emulationstation/es_systems.cfg, never /etc/emulationstation/es_systems.cfg
	static boost::filesystem::path getConfigPath(bool forWrite);
	static bool isValidSystemName(const std::string& name);
	static std::string getDatabasePath();
	static void writeExampleConfig(const boost::filesystem::path& path);

	void updateDatabase();
};
