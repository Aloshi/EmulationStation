#include "GamelistDB.h"
#include "MetaData.h"
#include "Log.h"
#include "SystemData.h"
#include <sstream>

namespace fs = boost::filesystem;

std::string pathToFileID(const fs::path& path, SystemData* system)
{
	return makeRelativePath(path, system->getStartPath(), true).generic_string();
}

fs::path fileIDToPath(const std::string& fileID, SystemData* system)
{
	return resolvePath(fileID, system->getStartPath(), true);
}

GamelistDB::GamelistDB(const std::string& path) : mDB(NULL)
{
	openDB(path.c_str());
	createTables();
}

GamelistDB::~GamelistDB()
{
	closeDB();
}

void GamelistDB::openDB(const char* path)
{
	if(sqlite3_open(path, &mDB))
	{
		throw DBException() << "Could not open database \"" << path << "\".\n"
			"\t" << sqlite3_errmsg(mDB);
	}
}

void GamelistDB::closeDB()
{
	if(mDB)
	{
		sqlite3_close(mDB);
		mDB = NULL;
	}
}

void GamelistDB::createTables()
{
	auto decl_type = GAME_METADATA;
	const std::vector<MetaDataDecl>& decl = getMDDMap().at(decl_type);

	std::stringstream ss;
	ss << "CREATE TABLE IF NOT EXISTS files (" <<
		"fileid VARCHAR(255), " <<
		"isfolder BOOLEAN NOT NULL, ";
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

	ss << "PRIMARY KEY (fileid))";

	if(sqlite3_exec(mDB, ss.str().c_str(), NULL, NULL, NULL))
		throw DBException() << "Error creating table!\n\t" << sqlite3_errmsg(mDB);
}

void populate_recursive(const fs::path& relativeTo, const std::vector<std::string>& extensions, const fs::path& start_dir, sqlite3* db, sqlite3_stmt* insert_stmt)
{
	bool contains; // ignored
	for(fs::directory_iterator end, dir(start_dir); dir != end; ++dir)
	{
		if(fs::is_directory(*dir))
		{
			populate_recursive(relativeTo, extensions, *dir, db, insert_stmt);
			continue;
		}

		fs::path path = *dir;
		path = removeCommonPath(path, relativeTo, contains).generic_string();

		if(std::find(extensions.begin(), extensions.end(), path.extension()) == extensions.end())
			continue;

		const std::string fileid_str = path.generic_string();
		if(sqlite3_bind_text(insert_stmt, 1, fileid_str.c_str(), fileid_str.size(), SQLITE_STATIC))
		{
			LOG(LogError) << "Error binding fileid in populate().\n\t" << sqlite3_errmsg(db);
			break;
		}

		if(sqlite3_step(insert_stmt) != SQLITE_DONE)
		{
			LOG(LogError) << "Error committing game \"" << fileid_str << "\" in populate().\n\t" << sqlite3_errmsg(db);
			break;
		}

		if(sqlite3_reset(insert_stmt))
		{
			LOG(LogError) << "Error resetting statement for \"" << fileid_str << "\" in populate().\n\t" << sqlite3_errmsg(db);
			break;
		}
	}
}

void GamelistDB::populate(const std::string& relativeTo, const std::vector<std::string>& extensions)
{
	const char* insert = "INSERT OR IGNORE INTO game (fileid) VALUES (?1)";

	sqlite3_stmt* stmt;
	if(sqlite3_prepare_v2(mDB, insert, strlen(insert), &stmt, NULL))
		throw DBException() << "Error preparing insert statement in populate().\n\t" << sqlite3_errmsg(mDB);

	if(sqlite3_exec(mDB, "BEGIN TRANSACTION", NULL, NULL, NULL))
	{
		sqlite3_finalize(stmt);
		throw DBException() << "Error beginning transaction in populate().\n\t" << sqlite3_errmsg(mDB);
	}

	// fill the game table
	populate_recursive(relativeTo, extensions, relativeTo, mDB, stmt);

	int err = sqlite3_exec(mDB, "COMMIT TRANSACTION", NULL, NULL, NULL);
	sqlite3_finalize(stmt);

	if(err)
		throw DBException() << "Error committing transaction in populate().\n\t" << sqlite3_errmsg(mDB);
}

void GamelistDB::prune(const std::string& relativeTo)
{
	sqlite3_stmt* readStmt;
	const char* read = "SELECT fileid FROM game";
	if(sqlite3_prepare_v2(mDB, read, strlen(read), &readStmt, NULL))
		throw DBException() << "Error preparing read statement.\n\t" << sqlite3_errmsg(mDB);

	sqlite3_stmt* removeStmt;
	const char* remove = "DELETE FROM game WHERE fileid = ?1";
	if(sqlite3_prepare_v2(mDB, remove, strlen(remove), &removeStmt, NULL))
	{
		sqlite3_finalize(readStmt);
		throw DBException() << "Error preparing delete statement.\n\t" << sqlite3_errmsg(mDB);
	}

	if(sqlite3_exec(mDB, "BEGIN TRANSACTION", NULL, NULL, NULL))
	{
		sqlite3_finalize(readStmt);
		sqlite3_finalize(removeStmt);
		throw DBException() << "Error beginning transaction.\n\t" << sqlite3_errmsg(mDB);
	}

	// for each game, check if its file exists - if it doesn't remove it from the DB
	while(sqlite3_step(readStmt) != SQLITE_DONE)
	{
		const char* path = (const char*)sqlite3_column_text(readStmt, 0);
		if(!fs::exists(relativeTo + "/" + path) && !fs::exists(path))
		{
			// doesn't exist, remove it
			LOG(LogInfo) << " Pruning game: \"" << relativeTo << "/" << path << "\"";
			sqlite3_bind_text(removeStmt, 1, path, strlen(path), SQLITE_STATIC);
			int err = sqlite3_step(removeStmt);
			if(err != SQLITE_DONE)
			{
				LOG(LogError) << "Error in prune() remove statement: " << sqlite3_errstr(err);
				break;
			}
			sqlite3_reset(removeStmt);
		}
	}

	int err = sqlite3_exec(mDB, "COMMIT TRANSACTION", NULL, NULL, NULL);
	sqlite3_finalize(readStmt);
	sqlite3_finalize(removeStmt);

	if(err)
		throw DBException() << "Error committing transaction.\n\t" << sqlite3_errmsg(mDB);
}

std::string GamelistDB::getValue(const std::string& fileID, const std::string& key) const
{
	sqlite3_stmt* readStmt;
	const char* read = "SELECT ?1 FROM files WHERE fileid = ?2";
	if(sqlite3_prepare_v2(mDB, read, strlen(read), &readStmt, NULL))
		throw DBException() << "Error preparing read statement.\n\t" << sqlite3_errmsg(mDB);

	sqlite3_bind_text(readStmt, 1, key.c_str(), key.size(), SQLITE_STATIC);
	sqlite3_bind_text(readStmt, 2, fileID.c_str(), fileID.size(), SQLITE_STATIC);

	if(sqlite3_step(readStmt) != SQLITE_DONE)
	{
		sqlite3_finalize(readStmt);
		throw DBException() << "Error reading file data (file \"" << fileID << "\")";
	}

	std::string ret = (const char*)sqlite3_column_text(readStmt, 0);
	sqlite3_finalize(readStmt);
	return ret;
}

MetaDataMap GamelistDB::getFileData(const std::string& fileID) const
{
	sqlite3_stmt* readStmt;
	const char* read = "SELECT * FROM files WHERE fileid = ?1";
	if(sqlite3_prepare_v2(mDB, read, strlen(read), &readStmt, NULL))
		throw DBException() << "Error preparing read statement.\n\t" << sqlite3_errmsg(mDB);

	sqlite3_bind_text(readStmt, 1, fileID.c_str(), fileID.size(), SQLITE_STATIC);

	if(sqlite3_step(readStmt) != SQLITE_DONE)
	{
		sqlite3_finalize(readStmt);
		throw DBException() << "Error reading file data (file \"" << fileID << "\")";
	}

	MetaDataListType type = sqlite3_column_int(readStmt, 1) ? FOLDER_METADATA : GAME_METADATA;
	MetaDataMap mdl(type);

	for(int i = 2; i < sqlite3_column_count(readStmt); i++)
	{
		const char* col = (const char*)sqlite3_column_name(readStmt, i);
		const char* value = (const char*)sqlite3_column_text(readStmt, i);

		mdl.set(col, value);
	}

	sqlite3_finalize(readStmt);

	return mdl;
}

void GamelistDB::setFileData(const std::string& fileID, const MetaDataMap& metadata)
{
	std::stringstream ss;
	ss << "INSERT OR REPLACE INTO files VALUES (?1, ?2, ";

	auto& mdd = metadata.getMDD();
	for(unsigned int i = 0; i < mdd.size(); i++)
	{
		ss << "?" << i + 3;

		if(i + 1 < mdd.size())
			ss << ", ";
	}
	ss << ")";

	sqlite3_stmt* stmt;
	std::string insert = ss.str();
	if(sqlite3_prepare_v2(mDB, insert.c_str(), insert.size(), &stmt, NULL))
		throw DBException() << "Error preparing insert statement: " << sqlite3_errmsg(mDB);
	
	sqlite3_bind_text(stmt, 1, fileID.c_str(), fileID.size(), SQLITE_STATIC); // fileid
	sqlite3_bind_int(stmt, 2, metadata.getType() == FOLDER_METADATA); // isfolder

	for(unsigned int i = 0; i < mdd.size(); i++)
	{
		const std::string& val = metadata.get(mdd.at(i).key);
		sqlite3_bind_text(stmt, i + 3, val.c_str(), val.size(), SQLITE_STATIC);
	}

	if(sqlite3_step(stmt) != SQLITE_DONE)
		LOG(LogError) << "Error inserting/replacing fileID \"" << fileID << "\".\n\t" << sqlite3_errmsg(mDB);

	sqlite3_finalize(stmt);
}

void GamelistDB::importXML(SystemData* system, const std::string& xml_path)
{
	LOG(LogInfo) << "Appending gamelist.xml file \"" << xml_path << "\" to database...";

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
	
	// set up SQLite stuff
	const char* insert = "INSERT OR IGNORE INTO game (fileid) VALUES (?1)";

	unsigned int skipCount = 0;
	const char* tagList[2] = { "game", "folder" };
	MetaDataListType typeList[2] = { GAME_METADATA, FOLDER_METADATA };

	for(int i = 0; i < 2; i++)
	{
		const char* tag = tagList[i];
		MetaDataListType type = typeList[i];

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
			MetaDataMap mdl(type);
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

					mdl.set(iter->key, value);
				}
			}

			// make sure the name is set
			assert(!mdl.get<std::string>("name").empty());

			this->setFileData(pathToFileID(path, system), mdl);
		}
	}
}

void GamelistDB::exportXML(SystemData* system, const std::string& xml_path)
{
	pugi::xml_document doc;
	pugi::xml_node root = doc.append_child("gameList");

	sqlite3_stmt* readStmt;
	const char* read = "SELECT * FROM files";
	if(sqlite3_prepare_v2(mDB, read, strlen(read), &readStmt, NULL))
		throw DBException() << "Error preparing read statement.\n\t" << sqlite3_errmsg(mDB);

	while(sqlite3_step(readStmt) != SQLITE_DONE)
	{
		MetaDataListType type = sqlite3_column_int(readStmt, 1) ? FOLDER_METADATA : GAME_METADATA;
		pugi::xml_node node = root.append_child(type == GAME_METADATA ? "game" : "folder");

		// write path
		node.append_child("path").text().set((const char*)sqlite3_column_text(readStmt, 0));

		// skip column 0 (fileid) and column 1 (isfolder)
		for(int i = 2; i < sqlite3_column_count(readStmt); i++)
		{
			const char* col = (const char*)sqlite3_column_name(readStmt, i);
			const char* value = (const char*)sqlite3_column_text(readStmt, i);
			node.append_child(col).text().set(value);
		}
	}

	sqlite3_finalize(readStmt);

	doc.save_file(xml_path.c_str());
}
