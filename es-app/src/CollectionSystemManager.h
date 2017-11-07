#pragma once

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"
#include "FileFilterIndex.h"
#include "SystemData.h"
#include "views/ViewController.h"

enum CollectionSystemType
{
	AUTO_ALL_GAMES,
	AUTO_LAST_PLAYED,
	AUTO_FAVORITES,
	CUSTOM_COLLECTION
};

struct CollectionSystemDecl
{
	CollectionSystemType type; // type of system
	std::string name;
	std::string longName;
	std::string defaultSort;
	std::string themeFolder;
	bool isCustom;
	bool isEditable;
};



class CollectionSystemManager
{
public:

	class CollectionSystem
	{
	public:
		virtual bool toggleGameInCollection(FileData* file);
		virtual void populateCollection(CollectionSystem* sysData);
		virtual void saveCollection(SystemData* sys) {};
		SystemData* system;
		CollectionSystemDecl decl;
		bool isEnabled;
		bool isPopulated;
		bool needsSave;
	};

	class AutoCollectionSystem : public CollectionSystem
	{
	public:
		bool toggleGameInCollection(FileData* file) override;
		void populateCollection(CollectionSystem* sysData) override;
		void init();
		void saveCollection(SystemData* sys) override;
		//private:

	};

	class CustomCollectionSystem : public CollectionSystem
	{
	public:
		bool toggleGameInCollection(FileData* file) override;
		void populateCollection(CollectionSystem* sysData) override;
		void init();
		void saveCollection(SystemData* sys);
		//private:

	};

	CollectionSystemManager(Window* window);
	~CollectionSystemManager();

	static CollectionSystemManager* get();
	static void init(Window* window);
	static void deinit();
	

	void loadCollectionSystems();
	void loadEnabledListFromSettings();
	void updateSystemsList();

	void refreshCollectionSystems(FileData* file);
	void updateCollectionSystem(FileData* file, CollectionSystem sysData);
	void deleteCollectionFiles(FileData* file);

	//inline std::map<std::string, CollectionSystem> getAutoCollectionSystems() { return mAutoCollectionSystemsData; };
	//inline std::map<std::string, CollectionSystem> getCustomCollectionSystems() { return mCustomCollectionSystemsData; };
	inline std::map<std::string, CollectionSystem> getCollectionSystems() { return mCollectionSystems; };

	std::vector<std::string> getEditableCollectionsNames();

	inline SystemData* getCustomCollectionsBundle() { return mCustomCollectionsBundle; };
	std::vector<std::string> getUnusedSystemsFromTheme();
	SystemData* addNewCustomCollection(std::string name);

	bool isThemeGenericCollectionCompatible(bool genericCustomCollections);
	bool isThemeCustomCollectionCompatible(std::vector<std::string> stringVector);
	std::string getValidNewCollectionName(std::string name, int index = 0);

	void setEditMode(std::string collectionName);
	void exitEditMode();
	inline bool isEditing() { return mIsEditingCustom; };
	inline std::string getEditingCollection() { return mEditingCollection; };
	bool toggleGameInCollection(FileData* file);

	SystemData* getSystemToView(SystemData* sys);
	void updateCollectionFolderMetadata(SystemData* sys);

private:
	static CollectionSystemManager* sInstance;
	SystemEnvironmentData* mCollectionEnvData;
	std::map<std::string, CollectionSystemDecl> mCollectionSystemDeclsIndex;
//	std::map<std::string, CollectionSystem> mAutoCollectionSystemsData;
//	std::map<std::string, CollectionSystem> mCustomCollectionSystemsData;
	std::map<std::string, CollectionSystem> mCollectionSystems;

	std::map<std::string, CollectionSystem> mEditableCollectionSystemsData;

	Window* mWindow;
	bool mIsEditingCustom;  // This bool denotes if user is editing any editable collection other than favs.
	std::string mEditingCollection;
	CollectionSystem* mEditingCollectionSystemData;

	void init();
	void initCustomCollectionSystems();
	SystemData* getAllGamesCollection();
	SystemData* createNewCollectionEntry(std::string name, CollectionSystemDecl sysDecl, bool index = true);
	void populateCustomCollection(CollectionSystem* sysData);

	void removeCollectionsFromDisplayedSystems();
	void addEnabledCollectionsToDisplayedSystems();

	std::vector<std::string> getSystemsFromConfig();
	std::vector<std::string> getSystemsFromTheme();
	std::vector<std::string> getCollectionsFromConfigFolder();
	std::vector<std::string> getCollectionThemeFolders(bool custom);
	std::vector<std::string> getUserCollectionThemeFolders();

	bool themeFolderExists(std::string folder);

	bool includeFileInAutoCollections(FileData* file);

	SystemData* mCustomCollectionsBundle;
};

std::string getCustomCollectionConfigPath(std::string collectionName);
std::string getCollectionsFolder();
bool systemSort(SystemData* sys1, SystemData* sys2);
