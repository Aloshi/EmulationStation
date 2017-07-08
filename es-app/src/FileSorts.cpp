#include "FileSorts.h"

namespace FileSorts
{
	const FileData::SortType typesArr[] = {
		FileData::SortType(&compareName, true, "filename, ascending"),
		FileData::SortType(&compareName, false, "filename, descending"),

		FileData::SortType(&compareRating, true, "rating, ascending"),
		FileData::SortType(&compareRating, false, "rating, descending"),

		FileData::SortType(&compareTimesPlayed, true, "times played, ascending"),
		FileData::SortType(&compareTimesPlayed, false, "times played, descending"),

		FileData::SortType(&compareLastPlayed, true, "last played, ascending"),
		FileData::SortType(&compareLastPlayed, false, "last played, descending"),

		FileData::SortType(&compareNumPlayers, true, "number players, ascending"),
		FileData::SortType(&compareNumPlayers, false, "number players, descending"),

		FileData::SortType(&compareReleaseDate, true, "release date, ascending"),
		FileData::SortType(&compareReleaseDate, false, "release date, descending"),

		FileData::SortType(&compareGenre, true, "genre, ascending"),
		FileData::SortType(&compareGenre, false, "genre, descending"),

		FileData::SortType(&compareDeveloper, true, "developer, ascending"),
		FileData::SortType(&compareDeveloper, false, "developer, descending"),

		FileData::SortType(&comparePublisher, true, "publisher, ascending"),
		FileData::SortType(&comparePublisher, false, "publisher, descending"),

		FileData::SortType(&compareSystem, true, "system, ascending"),
		FileData::SortType(&compareSystem, false, "system, descending")
	};

	const std::vector<FileData::SortType> SortTypes(typesArr, typesArr + sizeof(typesArr)/sizeof(typesArr[0]));

	//returns if file1 should come before file2
	bool compareName(const FileData* file1, const FileData* file2)
	{
		// we compare the actual metadata name, as collection files have the system appended which messes up the order
		std::string name1 = file1->metadata.get("name");
		std::string name2 = file2->metadata.get("name");
		transform(name1.begin(), name1.end(), name1.begin(), ::toupper);
		transform(name2.begin(), name2.end(), name2.begin(), ::toupper);
		return name1.compare(name2) < 0;
	}

	bool compareRating(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.getFloat("rating") < file2->metadata.getFloat("rating");
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
		// since it's stored as a POSIX string (YYYYMMDDTHHMMSS,fffffffff), we can compare as a string
		// as it's a lot faster than the time casts and then time comparisons
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return (file1)->metadata.get("lastplayed") < (file2)->metadata.get("lastplayed");
		}

		return false;
	}

	bool compareNumPlayers(const FileData* file1, const FileData* file2)
	{
		return (file1)->metadata.getInt("players") < (file2)->metadata.getInt("players");
	}

	bool compareReleaseDate(const FileData* file1, const FileData* file2)
	{
		return (file1)->metadata.getTime("releasedate") < (file2)->metadata.getTime("releasedate");
	}

	bool compareGenre(const FileData* file1, const FileData* file2)
	{
		std::string genre1 = file1->metadata.get("genre");
		std::string genre2 = file2->metadata.get("genre");
		transform(genre1.begin(), genre1.end(), genre1.begin(), ::toupper);
		transform(genre2.begin(), genre2.end(), genre2.begin(), ::toupper);
		return genre1.compare(genre2) < 0;
	}

	bool compareDeveloper(const FileData* file1, const FileData* file2)
	{
		std::string developer1 = file1->metadata.get("developer");
		std::string developer2 = file2->metadata.get("developer");
		transform(developer1.begin(), developer1.end(), developer1.begin(), ::toupper);
		transform(developer2.begin(), developer2.end(), developer2.begin(), ::toupper);
		return developer1.compare(developer2) < 0;
	}

	bool comparePublisher(const FileData* file1, const FileData* file2)
	{
		std::string publisher1 = file1->metadata.get("publisher");
		std::string publisher2 = file2->metadata.get("publisher");
		transform(publisher1.begin(), publisher1.end(), publisher1.begin(), ::toupper);
		transform(publisher2.begin(), publisher2.end(), publisher2.begin(), ::toupper);
		return publisher1.compare(publisher2) < 0;
	}

	bool compareSystem(const FileData* file1, const FileData* file2)
	{
		std::string system1 = file1->getSystemName();
		std::string system2 = file2->getSystemName();
		transform(system1.begin(), system1.end(), system1.begin(), ::toupper);
		transform(system2.begin(), system2.end(), system2.begin(), ::toupper);
		return system1.compare(system2) < 0;
	}
};
