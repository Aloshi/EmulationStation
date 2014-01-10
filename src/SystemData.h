#ifndef _SYSTEMDATA_H_
#define _SYSTEMDATA_H_

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
		const std::string& command, PlatformIds::PlatformId platformId = PlatformIds::PLATFORM_UNKNOWN);
	~SystemData();

	inline FileData* getRootFolder() const { return mRootFolder; };
	inline const std::string& getName() const { return mName; }
	inline const std::string& getFullName() const { return mFullName; }
	inline const std::string& getStartPath() const { return mStartPath; }
	inline const std::vector<std::string>& getExtensions() const { return mSearchExtensions; }
	inline PlatformIds::PlatformId getPlatformId() const { return mPlatformId; }
	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

	std::string getGamelistPath() const;
	bool hasGamelist() const;
	std::string getThemePath() const;
	
	unsigned int getGameCount() const;

	void launchGame(Window* window, FileData* game);

	static void deleteSystems();
	static bool loadConfig(const std::string& path, bool writeExampleIfNonexistant = true); //Load the system config file at getConfigPath(). Returns true if no errors were encountered. An example can be written if the file doesn't exist.
	static void writeExampleConfig(const std::string& path);
	static std::string getConfigPath();

	static std::vector<SystemData*> sSystemVector;

	inline std::vector<SystemData*>::const_iterator getIterator() const { return std::find(sSystemVector.begin(), sSystemVector.end(), this); };
	inline std::vector<SystemData*>::const_reverse_iterator getRevIterator() const { return std::find(sSystemVector.rbegin(), sSystemVector.rend(), this); };
	
	inline SystemData* getNext() const
	{
		auto it = getIterator();
		it++;
		if(it == sSystemVector.end()) it = sSystemVector.begin();
		return *it;
	}

	inline SystemData* getPrev() const
	{
		auto it = getRevIterator();
		it++;
		if(it == sSystemVector.rend()) it = sSystemVector.rbegin();
		return *it;
	}

	// Load or re-load theme.
	void loadTheme();

private:
	std::string mName;
	std::string mFullName;
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
	PlatformIds::PlatformId mPlatformId;
	std::shared_ptr<ThemeData> mTheme;

	void populateFolder(FileData* folder);

	FileData* mRootFolder;
};

#endif
