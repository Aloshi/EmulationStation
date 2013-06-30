#ifndef _FOLDER_H_
#define _FOLDER_H_

#include <map>
#include <vector>

#include "FileData.h"


class SystemData;

//This class lets us hold a vector of FileDatas within under a common name.
class FolderData : public FileData
{
public:
	typedef bool ComparisonFunction(const FileData* a, const FileData* b);
	struct SortState
	{
		ComparisonFunction & comparisonFunction;
		bool ascending;
        std::string description;

		SortState(ComparisonFunction & sortFunction, bool sortAscending, const std::string & sortDescription) : comparisonFunction(sortFunction), ascending(sortAscending), description(sortDescription) {}
	};

private:
	static std::map<ComparisonFunction*, std::string> sortStateNameMap;

public:
	FolderData(SystemData* system, std::string path, std::string name);
	~FolderData();

	bool isFolder() const;
	const std::string & getName() const;
	const std::string & getPath() const;

	unsigned int getFileCount();
	FileData* getFile(unsigned int i) const;
	std::vector<FileData*> getFiles(bool onlyFiles = false) const;
	std::vector<FileData*> getFilesRecursive(bool onlyFiles = false) const;

	void pushFileData(FileData* file);

	void sort(ComparisonFunction & comparisonFunction = compareFileName, bool ascending = true);
	static bool compareFileName(const FileData* file1, const FileData* file2);
	static bool compareRating(const FileData* file1, const FileData* file2);
	static bool compareUserRating(const FileData* file1, const FileData* file2);
	static bool compareTimesPlayed(const FileData* file1, const FileData* file2);
	static bool compareLastPlayed(const FileData* file1, const FileData* file2);
	static std::string getSortStateName(ComparisonFunction & comparisonFunction = compareFileName, bool ascending = true);

private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;
	std::vector<FileData*> mFileVector;
};

#endif
