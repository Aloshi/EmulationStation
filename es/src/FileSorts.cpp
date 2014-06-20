#include "FileSorts.h"

namespace FileSorts
{
	const FileData::SortType typesArr[] = {
		FileData::SortType(&compareFileName, true, "filename, ascending"),
		FileData::SortType(&compareFileName, false, "filename, descending"),

		FileData::SortType(&compareRating, true, "rating, ascending"),
		FileData::SortType(&compareRating, false, "rating, descending"),

		FileData::SortType(&compareTimesPlayed, true, "times played, ascending"),
		FileData::SortType(&compareTimesPlayed, false, "times played, descending"),

		FileData::SortType(&compareLastPlayed, true, "last played, ascending"),
		FileData::SortType(&compareLastPlayed, false, "last played, descending")
	};

	const std::vector<FileData::SortType> SortTypes(typesArr, typesArr + sizeof(typesArr)/sizeof(typesArr[0]));

	//returns if file1 should come before file2
	bool compareFileName(const FileData* file1, const FileData* file2)
	{
		std::string name1 = file1->getName();
		std::string name2 = file2->getName();

		//min of name1/name2 .length()s
		unsigned int count = name1.length() > name2.length() ? name2.length() : name1.length();
		for(unsigned int i = 0; i < count; i++)
		{
			if(toupper(name1[i]) != toupper(name2[i]))
			{
				return toupper(name1[i]) < toupper(name2[i]);
			}
		}

		return name1.length() < name2.length();
	}

	bool compareRating(const FileData* file1, const FileData* file2)
	{
		//only games have rating metadata
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return file1->metadata.getFloat("rating") < file2->metadata.getFloat("rating");
		}

		return false;
	}

	bool compareTimesPlayed(const FileData* file1, const FileData* file2)
	{
		//only games have playcount metadata
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return (file1)->metadata.getInt("playcount") < (file2)->metadata.getInt("playcount");
		}

		return false;
	}

	bool compareLastPlayed(const FileData* file1, const FileData* file2)
	{
		//only games have lastplayed metadata
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return (file1)->metadata.getTime("lastplayed") < (file2)->metadata.getTime("lastplayed");
		}

		return false;
	}
};
