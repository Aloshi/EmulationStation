#pragma once

#include <vector>
#include "FileData.h"
#include "SystemData.h"

namespace FileSorts
{
	bool compareName(const FileData* file1, const FileData* file2);
	bool compareRating(const FileData* file1, const FileData* file2);
	bool compareTimesPlayed(const FileData* file1, const FileData* fil2);
	bool compareLastPlayed(const FileData* file1, const FileData* file2);
	bool compareNumPlayers(const FileData* file1, const FileData* file2);
	bool compareReleaseDate(const FileData* file1, const FileData* file2);
	bool compareGenre(const FileData* file1, const FileData* file2);
	bool compareDeveloper(const FileData* file1, const FileData* file2);
	bool comparePublisher(const FileData* file1, const FileData* file2);
	bool compareSystem(const FileData* file1, const FileData* file2);

	extern const std::vector<FileData::SortType> SortTypes;
};
