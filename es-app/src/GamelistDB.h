#pragma once

#include "ESException.h"
#include "MetaData.h"
#include <string>
#include <sqlite3/sqlite3.h>

class SystemData;

class DBException : public ESException {};

boost::filesystem::path fileIDToPath(const std::string& fileID, SystemData* system);

/*
 Gamelist DB format:

 A separate table is made for each system (snes, nes, etc.), with these columns:

 [file ID] [system ID] [file type] [file exists] [metadata 0] [metadata 1] [metadata 2] ... etc.
 The primary key for this table is the pair (file ID, system ID).

 File ID is a string. File type is an int. Metadata types correspond to the MetaDataDecl vector.

 File ID is in the following format:
 ./path/to/game.rom  - for paths relative to system root
 ~/path/to/game.rom  - for paths relative to home
 /path/to/game.rom   - for absolute paths to game
 C:/path/to/game.rom - for absolute paths to game on Windows

 File ID is ALWAYS stored with forward slashes ("generic directory separators," as boost::filesystem calls them).

 File type is an enum value, where:
 0 - game
 1 - folder

 File exists is a boolean indicating whether or not the file is present on the file system.
 This value is set at startup.
*/


class GamelistDB
{
public:
	GamelistDB(const std::string& db_path);
	virtual ~GamelistDB();

	void addMissingFiles(const SystemData* system);
	void updateExists(const SystemData* system);
	// void pruneMissingFiles(const SystemData* system);

	// returns all metadata for a given fileID (basically "SELECT *" in SQL)
	MetaDataMap getFileData(const std::string& fileID, const std::string& systemID) const;

	// Sets all metadata for a given fileID (overwrites existing data).
	// If delete_existing is false, then any data that is not specified in data 
	// but already exists in the database will be left alone. If delete_existing is
	// true, the record for this fileID will be completely deleted (if it exists) 
	// before setting the new data.
	void setFileData(const std::string& fileID, const std::string& systemID, const MetaDataMap& metadata);

	void importXML(const SystemData* system, const std::string& xml_path);
	void exportXML(const SystemData* system, const std::string& xml_path);

private:
	void openDB(const char* path);
	void createTables();
	void closeDB();

	sqlite3* mDB;
};
