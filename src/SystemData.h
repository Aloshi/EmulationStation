#ifndef _SYSTEMDATA_H_
#define _SYSTEMDATA_H_

#include <vector>
#include <string>
#include "FolderData.h"
#include "Window.h"

class GameData;

class SystemData
{
public:
	SystemData(std::string name, std::string descName, std::string startPath, std::string extension, std::string command, std::string image);
	~SystemData();

	FolderData* getRootFolder();
	std::string getName();
	std::string getImage();
	std::string getDescName();
	std::string getStartPath();
	std::string getExtension();
	std::string getGamelistPath();
	
	bool hasGamelist();

	void launchGame(Window* window, GameData* game);

	static void deleteSystems();
	static void loadConfig();
	static void writeExampleConfig();
	static std::string getConfigPath();
	static std::string getBlankConsoleImagePath();

	static std::vector<SystemData*> sSystemVector;
private:
	std::string mName;
	std::string mDescName;
	std::string mStartPath;
	std::string mSearchExtension;
	std::string mLaunchCommand;
	std::string mImage;

	void populateFolder(FolderData* folder);

	FolderData* mRootFolder;
};

#endif
