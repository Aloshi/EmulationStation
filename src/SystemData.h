#ifndef _SYSTEMDATA_H_
#define _SYSTEMDATA_H_

#include <vector>
#include <string>
#include "FolderData.h"

class GameData;

class SystemData
{
public:
	SystemData(std::string name, std::string desc, std::string startPath, std::string extension, std::string command);
	~SystemData();

	FolderData* getRootFolder();
	std::string getName();
	std::string getDesc();
	std::string getStartPath();
	std::string getExtension();
	std::string getGamelistPath();
	bool hasGamelist();

	void launchGame(GameData* game);

	static void deleteSystems();
	static void loadConfig();
	static void writeExampleConfig();
	static std::string getConfigPath();

	static std::vector<SystemData*> sSystemVector;
private:
	std::string mName;
	std::string mDesc;
	std::string mStartPath;
	std::string mSearchExtension;
	std::string mLaunchCommand;

	void populateFolder(FolderData* folder);

	FolderData* mRootFolder;
};

#endif
