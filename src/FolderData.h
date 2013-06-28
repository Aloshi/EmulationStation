#ifndef _FOLDER_H_
#define _FOLDER_H_

#include "FileData.h"
#include <vector>

class SystemData;

//This class lets us hold a vector of FileDatas within under a common name.
class FolderData : public FileData
{
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

	void sort();

	static bool compareFileName(const FileData* file1, const FileData* file2);
	static bool compareRating(const FileData* file1, const FileData* file2);
	static bool compareUserRating(const FileData* file1, const FileData* file2);
	static bool compareTimesPlayed(const FileData* file1, const FileData* file2);
	static bool compareLastPlayed(const FileData* file1, const FileData* file2);
private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;
	std::vector<FileData*> mFileVector;
};

#endif
