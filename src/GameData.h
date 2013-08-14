#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include <string>

#include "FileData.h"
#include "SystemData.h"
#include "MetaData.h"

//This class holds information about a game: at the least, its name, system, and path. Additional information is optional and read by other classes.
class GameData : public FileData
{
public:
	GameData(SystemData* system, std::string path);

	const std::string& getName() const override;
	const std::string& getPath() const override;
	
	void incTimesPlayed();
	void lastPlayedNow();

	std::string getBashPath() const;
	std::string getBaseName() const;

	bool isFolder() const override;

	MetaDataList* metadata();

private:
	SystemData* mSystem;
	const std::string mPath;
	const std::string mBaseName;

	//extra data
	/*std::string mDescription;
	std::string mImagePath;
	float mRating;
	float mUserRating;
	size_t mTimesPlayed;
	std::time_t mLastPlayed;*/

	MetaDataList mMetaData;
};

#endif
