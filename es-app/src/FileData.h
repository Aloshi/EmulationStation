#pragma once

#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "MetaData.h"

class SystemData;
struct FileSort;

enum FileType
{
	GAME = 1,   // Cannot have children.
	FOLDER = 2
};

enum FileChangeType
{
	FILE_ADDED,
	FILE_METADATA_CHANGED,
	FILE_REMOVED,
	FILE_SORTED
};

// Used for loading/saving gamelist.xml.
const char* fileTypeToString(FileType type);
FileType stringToFileType(const char* str);

// Remove (.*) and [.*] from str
std::string removeParenthesis(const std::string& str);

std::string getCleanGameName(const std::string& str, const SystemData* system);

class FileData
{
public:
	FileData();
	FileData(const std::string& fileID, SystemData* system, const std::string& nameCache = "");
	FileData(const std::string& fileID, const std::string& systemID);

	inline bool operator==(const FileData& rhs) const { return (mFileID == rhs.mFileID && mSystem == rhs.mSystem); }
	inline bool operator!=(const FileData& rhs) const { return !(*this == rhs); }

	MetaDataMap get_metadata() const;
	void set_metadata(const MetaDataMap& metadata);

	const std::string& getName() const;
	FileType getType() const;

	boost::filesystem::path getPath() const;

	inline const std::string& getFileID() const { return mFileID; }
	const std::string& getSystemID() const;
	SystemData* getSystem() const { return mSystem; }

	std::vector<FileData> getChildren(const FileSort* sortType = NULL) const;
	std::vector<FileData> getChildrenRecursive(bool includeFolders, const FileSort* sortType = NULL) const;

	inline std::string getCleanName() const { return getCleanGameName(mFileID, mSystem); }

private:
	std::string mFileID;
	SystemData* mSystem;
	
	mutable std::string mNameCache;
	mutable FileType mTypeCache;
};
