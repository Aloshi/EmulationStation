#include "GamelistDB.h"
#include "MetaData.h"
#include "Log.h"
#include "SystemData.h"
#include <sstream>
#include <map>
#include <boost/assign.hpp>

namespace fs = boost::filesystem;

#define RESERVED_COLUMNS 4
#define COL_FILEID 0
#define COL_SYSTEMID 1
#define COL_FILETYPE 2
#define COL_FILEEXISTS 3

std::string pathToFileID(const fs::path& path, const fs::path& systemStartPath)
{
	return makeRelativePath(path, systemStartPath, false).generic_string();
}

std::string pathToFileID(const fs::path& path, const SystemData* system)
{
	return pathToFileID(path, system->getStartPath());
}

fs::path fileIDToPath(const std::string& fileID, const SystemData* system)
{
	return resolvePath(fileID, system->getStartPath(), true);
}

MetaDataListType fileTypeToMetaDataType(FileType type)
{
	switch(type)
	{
	case GAME:
		return GAME_METADATA;
	case FOLDER:
		return FOLDER_METADATA;
	}

	assert(false);
	return GAME_METADATA;
}

std::vector<FileSort> sFileSorts = boost::assign::list_of
	(FileSort("Alphabetical, asc", "ORDER BY name"))
	(FileSort("Alphabetical, desc", "ORDER BY name DESC"));

const std::vector<FileSort>& getFileSorts()
{
	return sFileSorts;
}

// super simple RAII wrapper for sqlite3
// prepared statement, can be used just like an sqlite3_stmt* thanks to overloaded operator
class SQLPreparedStmt
{
public:
	SQLPreparedStmt(sqlite3* db, const char* stmt) : mDB(db), mStmt(NULL) {
		if(sqlite3_prepare_v2(db, stmt, strlen(stmt), &mStmt, NULL))
			throw DBException() << "Error creating prepared stmt \"" << stmt << "\".\n\t" << sqlite3_errmsg(db);
	}

	SQLPreparedStmt(sqlite3* db, const std::string& stmt) : SQLPreparedStmt(db, stmt.c_str()) {};

	int step() { return sqlite3_step(mStmt); }

	void step_expected(int expected) {
		int result = step();
		if(result != expected)
			throw DBException() << "Step failed (got " << result << ", expected " << expected << "!\n\t" << sqlite3_errmsg(mDB);
	}

	void reset() { 
		if(sqlite3_reset(mStmt))
			throw DBException() << "Error resetting statement!\n\t" << sqlite3_errmsg(mDB);
	}

	~SQLPreparedStmt() {
		if(mStmt)
			sqlite3_finalize(mStmt);
	}

	operator sqlite3_stmt*() { return mStmt; }

private:
	sqlite3* mDB; // used for error messages
	sqlite3_stmt* mStmt;
};

// encapsulates a transaction that cannot outlive the lifetime of this object
class SQLTransaction
{
public:
	SQLTransaction(sqlite3* db) : mDB(db) {
		if(sqlite3_exec(mDB, "BEGIN TRANSACTION", NULL, NULL, NULL))
			throw DBException() << "Error beginning transaction.\n\t" << sqlite3_errmsg(mDB);
	}

	void commit() {
		if(!mDB)
			throw DBException() << "Transaction already committed!";

		if(sqlite3_exec(mDB, "COMMIT TRANSACTION", NULL, NULL, NULL))
			throw DBException() << "Error committing transaction.\n\t" << sqlite3_errmsg(mDB);

		mDB = NULL;
	}

	~SQLTransaction() {
		if(mDB)
			commit();
	}

private:
	sqlite3* mDB;
};


GamelistDB::GamelistDB(const std::string& path) : mDB(NULL)
{
	openDB(path.c_str());
}

GamelistDB::~GamelistDB()
{
	closeDB();
}

int match_start(const char* file, const char* dir)
{
	int i = 0;
	while(true)
	{
		// success
		if(dir[i] == '\0')
		{
			if(dir[i - 1] != '/')
			{
				if(file[i] == '/')
					i++;
				else
					return -1;
			}
			
			return i;
		}

		if(file[i] != dir[i])
			return -1;

		i++;
	}
}

bool has_slash(const char* file)
{
	int off = 0;
	while(file[off] != '\0')
	{
		if(file[off] == '/')
			return true;

		off++;
	}

	return false;
}

void sqlite_inimmediatedir(sqlite3_context* ctx, int argc, sqlite3_value** argv)
{
	int success = 0;

	if(argc == 2)
	{
		const char* file = (const char*)sqlite3_value_text(argv[0]);
		const char* dir = (const char*)sqlite3_value_text(argv[1]);

		if(file && dir)
		{
			int dir_end = match_start(file, dir);
			if(dir_end != -1 && !has_slash(file + dir_end))
			{
				success = 1;
			}
		}
	}

	sqlite3_result_int(ctx, success);
}

void sqlite_indir(sqlite3_context* ctx, int argc, sqlite3_value** argv)
{
	int success = 0;

	if(argc == 2)
	{
		const char* file = (const char*)sqlite3_value_text(argv[0]);
		const char* dir = (const char*)sqlite3_value_text(argv[1]);

		if(file && dir && match_start(file, dir) != -1)
			success = 1;
	}

	sqlite3_result_int(ctx, success);
}

void GamelistDB::openDB(const char* path)
{
	if(sqlite3_open(path, &mDB))
	{
		throw DBException() << "Could not open database \"" << path << "\".\n"
			"\t" << sqlite3_errmsg(mDB);
	}

	// register custom functions to handle directory comparisons
	if(sqlite3_create_function_v2(mDB, "inimmediatedir", 2, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, &sqlite_inimmediatedir, NULL, NULL, NULL))
		throw DBException() << "Could not register indir function.\n\t" << sqlite3_errmsg(mDB);
	if(sqlite3_create_function_v2(mDB, "indir", 2, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, &sqlite_indir, NULL, NULL, NULL))
		throw DBException() << "Could not register indir function.\n\t" << sqlite3_errmsg(mDB);

	createMissingTables();

	if(!hasValidSchema())
		recreateTables();
}

void GamelistDB::closeDB()
{
	if(mDB)
	{
		sqlite3_close(mDB);
		mDB = NULL;
	}
}

void GamelistDB::createMissingTables()
{
	const std::vector<MetaDataDecl>& decl = getMDDMap().at(GAME_METADATA);

	std::stringstream ss;
	ss << "CREATE TABLE IF NOT EXISTS files (" <<
		"fileid VARCHAR(255) NOT NULL, " <<
		"systemid VARCHAR(255) NOT NULL, " <<
		"filetype INT NOT NULL, " <<
		"fileexists BOOLEAN, ";
	for(auto it = decl.begin(); it != decl.end(); it++)
	{
		// format here is "[key] [type] DEFAULT [default_value],"
		ss << it->key << " ";

		// metadata type -> SQLite type
		switch(it->type)
		{
		case MD_IMAGE_PATH:
		case MD_MULTILINE_STRING:
		case MD_STRING:
			ss << "VARCHAR(255)";
			if(!it->defaultValue.empty())
				ss << " DEFAULT '" << it->defaultValue << "'";
			break;

		case MD_INT:
			ss << "INT";
			if(!it->defaultValue.empty())
				ss << " DEFAULT '" << it->defaultValue << "'";
			break;

		case MD_RATING:
		case MD_FLOAT:
			ss << "FLOAT";
			if(!it->defaultValue.empty())
				ss << " DEFAULT '" << it->defaultValue << "'";
			break;

		case MD_DATE:
			ss << "DATE"; // TODO default
			break;
		case MD_TIME:
			ss << "DATETIME"; // TODO default
			break;
		}

		ss << ", ";
	}

	ss << "PRIMARY KEY (fileid, systemid))";

	if(sqlite3_exec(mDB, ss.str().c_str(), NULL, NULL, NULL))
		throw DBException() << "Error creating table!\n\t" << sqlite3_errmsg(mDB);
}

// returns a vector of all columns in a particular table
// (order is preserved)
std::vector<std::string> get_columns(sqlite3* db, const std::string& table_name)
{
	std::vector<std::string> columns;

	std::string query = "PRAGMA table_info(" + table_name + ")";
	SQLPreparedStmt stmt(db, query);
	while(stmt.step() != SQLITE_DONE)
	{
		columns.push_back((const char*)sqlite3_column_text(stmt, 1));
	}

	return columns;
}

// returns a vector of all columns common to all supplied tables
// order is not preserved!
std::vector<std::string> get_common_columns(sqlite3* db, const std::vector<std::string>& table_names)
{
	std::map<std::string, int> columns;

	for(auto it = table_names.begin(); it != table_names.end(); it++)
	{
		std::string query = "PRAGMA table_info(" + *it + ")";
		SQLPreparedStmt stmt(db, query);
		while(stmt.step() != SQLITE_DONE)
		{
			columns[((const char*)sqlite3_column_text(stmt, 1))]++;
		}
	}

	std::vector<std::string> common;
	for(auto it = columns.begin(); it != columns.end(); it++)
	{
		if(it->second == table_names.size())
			common.push_back(it->first);
	}

	return common;
}

bool GamelistDB::hasValidSchema() const
{
	auto decl_type = GAME_METADATA;
	const std::vector<MetaDataDecl>& decl = getMDDMap().at(decl_type);

	std::vector<std::string> columns = get_columns(mDB, "files");
	if(columns.size() != RESERVED_COLUMNS + decl.size())
		return false;

	for(unsigned int i = 0; i < decl.size(); i++)
	{
		if(columns[RESERVED_COLUMNS + i] != decl.at(i).key)
		{
			LOG(LogInfo) << "Non-matching column #" << i << " (expected " << decl.at(i).key << ", got " << columns[RESERVED_COLUMNS + i] << ")";
			return false;
		}
	}

	return true;
}

void GamelistDB::recreateTables()
{
	LOG(LogInfo) << "Re-creating files table...";

	if(sqlite3_exec(mDB, "ALTER TABLE files RENAME TO files_old", NULL, NULL, NULL))
		throw DBException() << "Existing table could not be renamed!";

	createMissingTables();

	std::vector<std::string> common_cols = get_common_columns(mDB, { "files", "files_old" });
	std::stringstream ss;
	for(unsigned int i = 0; i < common_cols.size(); i++)
	{
		ss << common_cols.at(i) << (i + 1 < common_cols.size() ? ", " : "");
	}

	std::string copy = "INSERT INTO files (" + ss.str() + ") SELECT " + ss.str() + " FROM files_old";
	if(sqlite3_exec(mDB, copy.c_str(), NULL, NULL, NULL))
		throw DBException() << "Error copying into new table!";

	if(sqlite3_exec(mDB, "DROP TABLE files_old", NULL, NULL, NULL))
		throw DBException() << "Error dropping old table!";

	LOG(LogInfo) << "Recreated files table successfully!";
}

// used by populate_recursive to insert a single file into the database
// assumes insert_stmt already has system ID set, and that
// ?1 = fileid and ?2 = filetype
void add_file(const char* fileid, FileType filetype, const SystemData* system, sqlite3* db, sqlite3_stmt* insert_stmt)
{
	if(sqlite3_bind_text(insert_stmt, 1, fileid, strlen(fileid), SQLITE_STATIC))
		throw DBException() << "Error binding fileid in populate().\n\t" << sqlite3_errmsg(db);

	if(sqlite3_bind_int(insert_stmt, 2, filetype))
		throw DBException() << "Error binding filetype in populate().\n\t" << sqlite3_errmsg(db);

	std::string clean_name = getCleanGameName(fileid, system);
	if(sqlite3_bind_text(insert_stmt, 3, clean_name.c_str(), clean_name.size(), SQLITE_STATIC))
		throw DBException() << "Error binding filetype in populate().\n\t" << sqlite3_errmsg(db);

	if(sqlite3_step(insert_stmt) != SQLITE_DONE)
		throw DBException() << "Error adding file \"" << fileid << "\" in populate().\n\t" << sqlite3_errmsg(db);

	if(sqlite3_reset(insert_stmt))
		throw DBException() << "Error resetting statement for \"" << fileid << "\" in populate().\n\t" << sqlite3_errmsg(db);
}

// what this does:
// - if we add at least one valid file in this folder, return true.
// - given a folder (start_dir), go through all the files and folders in it.
// - if it's a file, check if its extension matches our list (extensions),
//   adding it to the "files" table if it does (filetype = game).
//   also mark this folder as having a file.
// - if it's a folder, recurse into it. if that recursion returns true, also mark this folder as having a file.
// - if this folder is marked as having a file at return time, add it to the database.

bool populate_recursive(const fs::path& relativeTo, const std::vector<std::string>& extensions, 
	const fs::path& start_dir, const SystemData* system, sqlite3* db, sqlite3_stmt* insert_stmt)
{
	// make sure that this isn't a symlink to a thing we already have
	if(fs::is_symlink(start_dir))
	{
		// if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if(start_dir.generic_string().find(fs::canonical(start_dir).generic_string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << start_dir << "\"";
			return false;
		}
	}

	bool has_a_file = false;
	for(fs::directory_iterator end, dir(start_dir); dir != end; ++dir)
	{
		// fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
		// see issue #75: https://github.com/Aloshi/EmulationStation/issues/75

		fs::path path = *dir;

		// if the extension is on our list
		if(std::find(extensions.begin(), extensions.end(), path.extension().string()) != extensions.end())
		{
			// yep, it's a game: add it
			const std::string fileid = pathToFileID(path, relativeTo);
			add_file(fileid.c_str(), FileType::GAME, system, db, insert_stmt);
			has_a_file = true;
		}else{
			// it's not a game, if it's a directory check if it contains any games
			if(fs::is_directory(*dir))
			{
				// if it did have some games, add this directory to the DB later
				if(populate_recursive(relativeTo, extensions, *dir, system, db, insert_stmt))
					has_a_file = true;
			}
		}
	}

	if(has_a_file)
	{
		// this folder had a game, add it to the DB
		std::string fileid = pathToFileID(start_dir, relativeTo);
		add_file(fileid.c_str(), FileType::FOLDER, system, db, insert_stmt);
	}

	return has_a_file;
}

void GamelistDB::addMissingFiles(const SystemData* system)
{
	const std::string& relativeTo = system->getStartPath(); 
	const std::vector<std::string>& extensions = system->getExtensions();

	// ?1 = fileid, ?2 = filetype, ?3 = systemid
	SQLPreparedStmt stmt(mDB, "INSERT OR IGNORE INTO files (fileid, systemid, filetype, fileexists, name) VALUES (?1, ?4, ?2, 1, ?3)");
	sqlite3_bind_text(stmt, 4, system->getName().c_str(), system->getName().size(), SQLITE_STATIC);

	SQLTransaction transaction(mDB);

	// actually start adding things
	populate_recursive(relativeTo, extensions, relativeTo, system, mDB, stmt);

	transaction.commit();
}

void GamelistDB::updateExists(const SystemData* system)
{
	const std::string& relativeTo = system->getStartPath();

	SQLPreparedStmt readStmt(mDB, "SELECT fileid FROM files WHERE systemid = ?1");
	sqlite3_bind_text(readStmt, 1, system->getName().c_str(), system->getName().size(), SQLITE_STATIC);
	
	SQLPreparedStmt updateStmt(mDB, "UPDATE files SET fileexists = ?1 WHERE fileid = ?2 AND systemid = ?3");
	sqlite3_bind_text(updateStmt, 3, system->getName().c_str(), system->getName().size(), SQLITE_STATIC);

	SQLTransaction transaction(mDB);

	// for each game, check if its file exists - if it doesn't remove it from the DB
	while(readStmt.step() != SQLITE_DONE)
	{
		const char* path = (const char*)sqlite3_column_text(readStmt, 0);

		bool exists = false;
		if(path && path[0] == '.') // it's relative
			exists = fs::exists(relativeTo + "/" + path);
		else
			exists = fs::exists(path);

		sqlite3_bind_text(updateStmt, 2, path, strlen(path), SQLITE_STATIC);
		sqlite3_bind_int(updateStmt, 1, exists);
		updateStmt.step_expected(SQLITE_DONE);
		updateStmt.reset();
	}

	transaction.commit();
}

void GamelistDB::updateExists(const FileData& file)
{
	SQLPreparedStmt stmt(mDB, "UPDATE files SET fileexists = ?1 WHERE fileid = ?2 AND systemid = ?3");
	bool exists = fs::exists(fileIDToPath(file.getFileID(), file.getSystem()));
	sqlite3_bind_int(stmt, 1, exists);
	sqlite3_bind_text(stmt, 2, file.getFileID().c_str(), file.getFileID().size(), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, file.getSystem()->getName().c_str(), file.getSystem()->getName().size(), SQLITE_STATIC);
	stmt.step_expected(SQLITE_DONE);
}

MetaDataMap GamelistDB::getFileData(const std::string& fileID, const std::string& systemID) const
{
	SQLPreparedStmt readStmt(mDB, "SELECT * FROM files WHERE fileid = ?1 AND systemid = ?2");
	sqlite3_bind_text(readStmt, 1, fileID.c_str(), fileID.size(), SQLITE_STATIC);
	sqlite3_bind_text(readStmt, 2, systemID.c_str(), systemID.size(), SQLITE_STATIC);

	readStmt.step_expected(SQLITE_ROW);

	MetaDataListType type = fileTypeToMetaDataType((FileType)sqlite3_column_int(readStmt, COL_FILETYPE));
	MetaDataMap mdl(type);

	for(int i = RESERVED_COLUMNS; i < sqlite3_column_count(readStmt); i++)
	{
		const char* col = (const char*)sqlite3_column_name(readStmt, i);
		const char* value = (const char*)sqlite3_column_text(readStmt, i);

		if(value == NULL)
			value = "";

		mdl.set(col, value);
	}

	return mdl;
}

void GamelistDB::setFileData(const std::string& fileID, const std::string& systemID, FileType type, const MetaDataMap& metadata)
{
	std::stringstream ss;
	ss << "INSERT OR REPLACE INTO files VALUES (?1, ?2, ?3, ?4, ";

	auto& mdd = metadata.getMDD();
	for(unsigned int i = 0; i < mdd.size(); i++)
	{
		ss << "?" << i + RESERVED_COLUMNS + 1;

		if(i + 1 < mdd.size())
			ss << ", ";
	}
	ss << ")";

	std::string insertstr = ss.str();
	SQLPreparedStmt stmt(mDB, insertstr.c_str());
	sqlite3_bind_text(stmt, 1, fileID.c_str(), fileID.size(), SQLITE_STATIC); // fileid
	sqlite3_bind_text(stmt, 2, systemID.c_str(), systemID.size(), SQLITE_STATIC); // systemid
	sqlite3_bind_int(stmt, 3, type); // filetype
	sqlite3_bind_int(stmt, 4, 1); // fileexists

	for(unsigned int i = 0; i < mdd.size(); i++)
	{
		const std::string& val = metadata.get(mdd.at(i).key);
		sqlite3_bind_text(stmt, i + RESERVED_COLUMNS + 1, val.c_str(), val.size(), SQLITE_STATIC);
	}

	stmt.step_expected(SQLITE_DONE);
}

std::vector<FileData> GamelistDB::getChildrenOf(const std::string& fileID, SystemData* system, 
	bool immediateChildrenOnly, bool includeFolders, const FileSort* sortType)
{
	const std::string& systemID = system->getName();
	std::vector<FileData> children;

	std::stringstream ss;
	ss << "SELECT fileid, name, filetype FROM files WHERE systemid = ?1 AND fileexists = 1 ";
	if(immediateChildrenOnly)
		ss << "AND inimmediatedir(fileid, ?2) ";
	else
		ss << "AND indir(fileid, ?2) ";

	if(!includeFolders)
		ss << "AND NOT filetype = " << FileType::FOLDER << " ";

	if(sortType)
		ss << sortType->sql;
	else
		ss << "ORDER BY name";

	std::string query = ss.str();
	SQLPreparedStmt stmt(mDB, query);
	sqlite3_bind_text(stmt, 1, systemID.c_str(), systemID.size(), SQLITE_STATIC); // systemid
	sqlite3_bind_text(stmt, 2, fileID.c_str(), fileID.size(), SQLITE_STATIC);

	while(stmt.step() != SQLITE_DONE)
	{
		const char* fileid = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		FileType filetype = (FileType)sqlite3_column_int(stmt, 2);
		children.push_back(FileData(fileid, system, filetype, name ? name : ""));
	}

	return children;
}

void GamelistDB::importXML(const SystemData* system, const std::string& xml_path)
{
	LOG(LogInfo) << "Appending gamelist.xml file \"" << xml_path << "\" to database (system: " << system->getName() << ")...";

	if(!fs::exists(xml_path))
		throw ESException() << "XML file not found (path: " << xml_path << ")";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xml_path.c_str());
	if(!result)
		throw ESException() << "Error parsing XML:\n\t" << result.description();

	pugi::xml_node root = doc.child("gameList");
	if(!root)
		throw ESException() << "Could not find <gameList> node!";

	const fs::path relativeTo = system->getStartPath();
	
	unsigned int skipCount = 0;
	const char* tagList[2] = { "game", "folder" };
	MetaDataListType metadataTypeList[2] = { GAME_METADATA, FOLDER_METADATA };
	FileType fileTypeList[2] = { GAME, FOLDER };

	for(int i = 0; i < 2; i++)
	{
		const char* tag = tagList[i];
		MetaDataListType metadataType = metadataTypeList[i];
		FileType fileType = fileTypeList[i];

		for(pugi::xml_node fileNode = root.child(tag); fileNode; fileNode = fileNode.next_sibling(tag))
		{
			fs::path path = resolvePath(fileNode.child("path").text().get(), relativeTo, false);

			if(!boost::filesystem::exists(path))
			{
				LOG(LogWarning) << "File \"" << path << "\" does not exist! Ignoring.";
				skipCount++;
				continue;
			}

			// make a metadata map
			MetaDataMap mdl(metadataType);
			const std::vector<MetaDataDecl>& mdd = mdl.getMDD();
			for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
			{
				pugi::xml_node md = fileNode.child(iter->key.c_str());
				if(md)
				{
					// if it's a path, resolve relative paths
					std::string value = md.text().get();
					if(iter->type == MD_IMAGE_PATH)
						value = resolvePath(value, relativeTo, true).generic_string();
					
					// if it's a time/date, convert it into the SQLite format
					if(iter->type == MD_TIME || iter->type == MD_DATE)
						value = ptime_to_string(string_to_ptime(value, LEGACY_TIME_STRING_FORMAT), SQLITE_TIME_STRING_FORMAT);

					mdl.set(iter->key, value);
				}
			}

			const std::string fileID = pathToFileID(path, system);

			// make sure the name is set
			if(mdl.get<std::string>("name").empty())
				mdl.set<std::string>("name", getCleanGameName(fileID, system));

			this->setFileData(fileID, system->getName(), fileType, mdl);
		}
	}
}

void GamelistDB::exportXML(const SystemData* system, const std::string& xml_path)
{
	pugi::xml_document doc;
	pugi::xml_node root = doc.append_child("gameList");

	SQLPreparedStmt readStmt(mDB, "SELECT * FROM files WHERE systemid = ?1");
	sqlite3_bind_text(readStmt, 1, system->getName().c_str(), system->getName().size(), SQLITE_STATIC);
	
	std::string relativeTo = system->getStartPath();
	while(readStmt.step() != SQLITE_DONE)
	{
		MetaDataListType type = fileTypeToMetaDataType((FileType)sqlite3_column_int(readStmt, COL_FILETYPE));
		pugi::xml_node node = root.append_child(type == GAME_METADATA ? "game" : "folder");

		// write path
		std::string path = (const char*)sqlite3_column_text(readStmt, COL_FILEID);
		if(path[0] == '.')
			path = relativeTo + path.substr(1, std::string::npos);

		node.append_child("path").text().set(path.c_str());

		const auto& mdd = getMDDMap().at(type);

		// skip column 0 (fileid), 1 (systemid), 2 (filetype), 3 (fileexists)
		std::string temp;
		for(int i = RESERVED_COLUMNS; i < sqlite3_column_count(readStmt); i++)
		{
			const char* col = (const char*)sqlite3_column_name(readStmt, i);
			const char* value = (const char*)sqlite3_column_text(readStmt, i);
			
			for(auto it = mdd.begin(); it != mdd.end(); it++)
			{
				if(it->key == col)
				{
					// convert from SQLite time format to legacy gamelist.xml time format
					if(it->type == MD_TIME || it->type == MD_DATE)
					{
						temp = ptime_to_string(string_to_ptime(value, SQLITE_TIME_STRING_FORMAT), LEGACY_TIME_STRING_FORMAT);
						value = temp.c_str();
					}
					break;
				}
			}

			node.append_child(col).text().set(value);
		}
	}

	doc.save_file(xml_path.c_str());
}
