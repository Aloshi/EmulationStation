#pragma once

#include <vector>
#include "FileData.h"

namespace FileSorts
{
	bool compareFileName(const FileData* file1, const FileData* file2);
	bool compareRating(const FileData* file1, const FileData* file2);
	bool compareTimesPlayed(const FileData* file1, const FileData* fil2);
	bool compareLastPlayed(const FileData* file1, const FileData* file2);

	extern const std::vector<FileData::SortType> SortTypes;
};
