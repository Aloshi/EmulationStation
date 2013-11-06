#pragma once

#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "MetaData.h"

enum FileType
{
	GAME = 1,   // Cannot have children.
	FOLDER = 2
};

// Used for loading/saving gamelist.xml.
const char* fileTypeToString(FileType type);
FileType stringToFileType(const char* str);

std::string getCleanFileName(const boost::filesystem::path& path);

// A tree node that holds information for a file.
class FileData
{
public:
	FileData(FileType type, const boost::filesystem::path& path);
	virtual ~FileData();

	inline const std::string& getName() const { return metadata.get("name"); }
	inline FileType getType() const { return mType; }
	inline const boost::filesystem::path& getPath() const { return mPath; }
	inline FileData* getParent() const { return mParent; }
	inline const std::vector<FileData*>& getChildren() const { return mChildren; }
	
	virtual const std::string& getThumbnailPath() const;

	std::vector<FileData*> getFilesRecursive(unsigned int typeMask) const;

	void addChild(FileData* file); // Error if mType != FOLDER
	void removeChild(FileData* file); //Error if mType != FOLDER


	typedef bool ComparisonFunction(const FileData* a, const FileData* b);
	struct SortType
	{
		ComparisonFunction* comparisonFunction;
		bool ascending;
		std::string description;

		SortType(ComparisonFunction* sortFunction, bool sortAscending, const std::string & sortDescription) 
			: comparisonFunction(sortFunction), ascending(sortAscending), description(sortDescription) {}
	};

	void sort(ComparisonFunction& comparator, bool ascending = true);
	void sort(const SortType& type);

	MetaDataList metadata;

private:
	FileType mType;
	boost::filesystem::path mPath;
	FileData* mParent;
	std::vector<FileData*> mChildren;
};
