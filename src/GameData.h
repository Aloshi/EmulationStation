#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include <string>

#include "FileData.h"
#include "MetaData.h"

//This class holds information about a game: at the least, its name, system, and path. Additional information is optional and read by other classes.
class GameData : public FileData
{
public:
	GameData(const std::string& path, const MetaDataList& metadata);

	const std::string& getName() const override;
	const std::string& getPath() const override;
	
	void incTimesPlayed();
	void lastPlayedNow();

	std::string getBashPath() const;
	std::string getBaseName() const;
	std::string getCleanName() const;

	bool isFolder() const override;

	MetaDataList* metadata();

private:
	const std::string mPath;
	const std::string mBaseName;

	MetaDataList mMetaData;
};

#endif
