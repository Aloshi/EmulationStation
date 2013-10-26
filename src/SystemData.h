#ifndef _SYSTEMDATA_H_
#define _SYSTEMDATA_H_

#include <vector>
#include <string>
#include "FolderData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"

class GameData;

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
		const std::string& command, const std::string &emulatorScreenshotDumpDir, const std::string &screenshotDir, PlatformIds::PlatformId platformId = PlatformIds::PLATFORM_UNKNOWN);
	~SystemData();

	FolderData* getRootFolder();

	std::string getName();
	std::string getFullName();
	std::string getStartPath();
	std::string getGamelistPath();
        std::string getScreenshotDir();
        std::string getEmulatorScreenshotDumpDir();
	std::vector<std::string> getExtensions();
	PlatformIds::PlatformId getPlatformId();

	bool hasGamelist();
	std::vector<MetaDataDecl> getGameMDD();

	unsigned int getGameCount();

	void launchGame(Window* window, GameData* game);

	static void deleteSystems();
	static bool loadConfig(const std::string& path, bool writeExampleIfNonexistant = true); //Load the system config file at getConfigPath(). Returns true if no errors were encountered. An example can be written if the file doesn't exist.
	static void writeExampleConfig(const std::string& path);
	static std::string getConfigPath();

	static std::vector<SystemData*> sSystemVector;
private:
	std::string mName;
	std::string mFullName;
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
        std::string mScreenshotDir;
        std::string mEmulatorScreenshotDumpDir;
	PlatformIds::PlatformId mPlatformId;

	void populateFolder(FolderData* folder);

	FolderData* mRootFolder;
};

#endif

