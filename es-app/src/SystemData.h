#pragma once

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"
#include "SystemOption.h"

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions, 
		const std::string& command, const std::vector<PlatformIds::PlatformId>& platformIds, const std::string& themeFolder);
	virtual ~SystemData();

	inline const std::string& getName() const { return mName; }
	inline const std::string& getFullName() const { return mFullName; }
	inline const std::string& getStartPath() const { return mStartPath; }
	inline const std::vector<std::string>& getExtensions() const { return mSearchExtensions; }
	inline const std::vector<PlatformIds::PlatformId>& getPlatformIds() const { return mPlatformIds; }
	inline bool hasPlatformId(PlatformIds::PlatformId id) const { return std::find(mPlatformIds.begin(), mPlatformIds.end(), id) != mPlatformIds.end(); }
	inline const std::string& getThemeFolder() const { return mThemeFolder; }
	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

	bool hasOptions() const { return mOptions.size() > 0; }
	const std::vector<SystemOption*>& getOptions() const { return mOptions; }
	void addOption( SystemOption* option ) { mOptions.push_back( option ); }
	std::string getOption( std::string id, FileData *game );

	inline const FileData& getRootFolder() const { return mRoot; }

	std::string getThemePath() const;
	
	unsigned int getGameCount() const;
	bool hasFileWithImage() const;

	std::string getGamelistPath(bool forWrite) const;
	std::string replaceOptions( std::string command, FileData game ) const;

	void launchGame(Window* window, FileData game) const;

	// Load or re-load theme.
	void loadTheme();

private:
	std::string mName;
	std::string mFullName;
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
	std::vector<PlatformIds::PlatformId> mPlatformIds;
	std::string mThemeFolder;
	std::shared_ptr<ThemeData> mTheme;
	std::vector<SystemOption*> mOptions;

	FileData mRoot;
};
