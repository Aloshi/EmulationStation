#ifndef _SYSTEMDATA_H_
#define _SYSTEMDATA_H_

#include <vector>
#include <string>

class GameData;

class SystemData
{
public:
	SystemData(std::string name, std::string startPath, std::string extension);
	~SystemData();

	unsigned int getGameCount();
	GameData* getGame(unsigned int i);
	std::string getName();
private:
	std::string mName;
	std::string mStartPath;
	std::string mSearchExtension;
	std::vector<GameData*> mGameVector;
	void buildGameList();
};

#endif
