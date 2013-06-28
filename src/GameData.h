#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include <string>
#include "FileData.h"
#include "SystemData.h"

//This class holds information about a game: at the least, its name, system, and path. Additional information is optional and read by other classes.
class GameData : public FileData
{
public:
	//static tag names for reading/writing XML documents. This might fail in PUGIXML_WCHAR_MODE
	//TODO: The class should have member to read fromXML() and write toXML() probably...
	static const std::string xmlTagGameList;
	static const std::string xmlTagGame;
	static const std::string xmlTagName;
	static const std::string xmlTagPath;
	static const std::string xmlTagDescription;
	static const std::string xmlTagImagePath;
	static const std::string xmlTagRating;
	static const std::string xmlTagTimesPlayed;

	GameData(SystemData* system, std::string path, std::string name);

	const std::string & getName() const;
	void setName(const std::string & name);

	const std::string & getPath() const;
	void setPath(const std::string & path);

	const std::string & getDescription() const;
	void setDescription(const std::string & description);

	const std::string & getImagePath() const;
	void setImagePath(const std::string & imagePath);

	float getRating() const;
	void setRating(float rating);

	size_t getTimesPlayed() const;
	void setTimesPlayed(size_t timesPlayed);

	std::string getBashPath() const;
	std::string getBaseName() const;

	bool isFolder() const;
private:
	SystemData* mSystem;
	std::string mPath;
	std::string mName;

	//extra data
	std::string mDescription;
	std::string mImagePath;
	float mRating;
	size_t mTimesPlayed;
};

#endif
