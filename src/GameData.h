#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include <string>
#include "FileData.h"
#include "SystemData.h"

//This class holds information about a game: at the least, its name, system, and path. Additional information is optional and read by other classes.
class GameData : public FileData
{
public:
	GameData(SystemData* system, std::string path, std::string name);

	void set(std::string name = "", std::string description = "", std::string imagePath = "");

	std::string getName();
	std::string getPath();
	std::string getBashPath();
	std::string getBaseName();

	std::string getDescription();
	std::string getImagePath();

	bool isFolder();
private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;

	//extra data
	std::string mDescription;
	std::string mImagePath;
};

#endif
