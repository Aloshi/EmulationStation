#pragma once

#include "ESException.h"
#include "MetaData.h"
#include <string>
#include <sqlite3/sqlite3.h>

class SystemData;

class DBException : public ESException {};

boost::filesystem::path fileIDToPath(const std::string& fileID, SystemData* system);

class GamelistDB
{
public:
	GamelistDB(const std::string& path);
	virtual ~GamelistDB();

	void populate(const std::string& relativeTo, const std::vector<std::string>& extensions);
	void prune(const std::string& relativeTo);

	// returns a particular piece of metadata for fileID
	std::string getValue(const std::string& fileID, const std::string& key) const;
	
	void setValue(const std::string& fileID, const std::string& key, const std::string& value);

	// returns all metadata for a given fileID (basically "SELECT *" in SQL)
	MetaDataMap getFileData(const std::string& fileID) const;

	// Sets all metadata for a given fileID (overwrites existing data).
	// If delete_existing is false, then any data that is not specified in data 
	// but already exists in the database will be left alone. If delete_existing is
	// true, the record for this fileID will be completely deleted (if it exists) 
	// before setting the new data.
	void setFileData(const std::string& fileID, const MetaDataMap& metadata);

	void importXML(SystemData* system, const std::string& xml_path);
	void exportXML(SystemData* system, const std::string& xml_path);

private:
	void openDB(const char* path);
	void createTables();
	void closeDB();

	sqlite3* mDB;
};
