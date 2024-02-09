#pragma once
#ifndef ES_APP_COLLECTION_SYSTEM_MANAGER_H
#define ES_APP_COLLECTION_SYSTEM_MANAGER_H

#include <map>
#include <SDL_timer.h>
#include <string>
#include <vector>

class FileData;
class SystemData;
class Window;
struct SystemEnvironmentData;
class FileFilterIndex;

static const std::string CUSTOM_COLL_ID = "collections";
static const std::string RANDOM_COLL_ID = "random";
constexpr int LAST_PLAYED_MAX = 50;

constexpr int RANDOM_SYSTEM_MAX = 5;
constexpr int DEFAULT_RANDOM_SYSTEM_GAMES = 1;
constexpr int DEFAULT_RANDOM_COLLECTIONS_GAMES = 0;

enum CollectionSystemType
{
	AUTO_ALL_GAMES,
	AUTO_LAST_PLAYED,
	AUTO_FAVORITES,
	AUTO_RANDOM,
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
};

struct CollectionSystemData
{
	SystemData* system;
	CollectionSystemDecl decl;
	bool isEnabled;
	bool isPopulated;
	bool needsSave;
};

class CollectionSystemManager
{
public:
	CollectionSystemManager(Window* window);
	~CollectionSystemManager();

	static CollectionSystemManager* get();
	static void init(Window* window);
	static void deinit();
	void saveCustomCollection(SystemData* sys);

	void loadCollectionSystems(bool async=false);
	void loadEnabledListFromSettings();
	void updateSystemsList();

	void refreshCollectionSystems(FileData* file);
	void updateCollectionSystem(FileData* file, CollectionSystemData sysData);
	void deleteCollectionFiles(FileData* file);
	void recreateCollection(SystemData* sysData);

	inline std::map<std::string, CollectionSystemData> getAutoCollectionSystems() { return mAutoCollectionSystemsData; };
	inline std::map<std::string, CollectionSystemData> getCustomCollectionSystems() { return mCustomCollectionSystemsData; };
	inline SystemData* getCustomCollectionsBundle() { return mCustomCollectionsBundle; };
	inline SystemData* getRandomCollection() { return mRandomCollection; };
	std::vector<std::string> getUnusedSystemsFromTheme();
	SystemData* addNewCustomCollection(std::string name);

	bool isThemeGenericCollectionCompatible(bool genericCustomCollections);
	bool isThemeCustomCollectionCompatible(std::vector<std::string> stringVector);
	std::string getValidNewCollectionName(std::string name, int index = 0);

	void setEditMode(std::string collectionName, bool quiet = false);
	void exitEditMode(bool quiet = false);
	inline bool isEditing() { return mIsEditingCustom; };
	inline std::string getEditingCollection() { return mEditingCollection; };
	bool toggleGameInCollection(FileData* file);

	SystemData* getSystemToView(SystemData* sys);
	void updateCollectionFolderMetadata(SystemData* sys);

	SystemData* getAllGamesCollection();

	void trimCollectionCount(FileData* rootFolder, int limit, bool shuffle);

private:
	static CollectionSystemManager* sInstance;
	SystemEnvironmentData* mCollectionEnvData;
	std::map<std::string, CollectionSystemDecl> mCollectionSystemDeclsIndex;
	std::map<std::string, CollectionSystemData> mAutoCollectionSystemsData;
	std::map<std::string, CollectionSystemData> mCustomCollectionSystemsData;
	Window* mWindow;
	bool mIsEditingCustom;
	std::string mEditingCollection;
	CollectionSystemData* mEditingCollectionSystemData;
	Uint32 mFirstPressMs = 0;

	void initAutoCollectionSystems();
	void initCustomCollectionSystems();
	SystemData* createNewCollectionEntry(std::string name, CollectionSystemDecl sysDecl, bool index = true);
	void populateAutoCollection(CollectionSystemData* sysData);
	void populateCustomCollection(CollectionSystemData* sysData);
	void addRandomGames(SystemData* newSys, SystemData* sourceSystem, FileData* rootFolder, FileFilterIndex* index,
		std::map<std::string, std::map<std::string, int>> mapsForRandomColl, int defaultValue);
	void populateRandomCollectionFromCollections(std::map<std::string, std::map<std::string, int>> mapsForRandomColl);

	void removeCollectionsFromDisplayedSystems();
	void addEnabledCollectionsToDisplayedSystems(std::map<std::string, CollectionSystemData>* colSystemData, bool processRandom);

	std::vector<std::string> getSystemsFromConfig();
	std::vector<std::string> getSystemsFromTheme();
	std::vector<std::string> getCollectionsFromConfigFolder();
	std::vector<std::string> getCollectionThemeFolders(bool custom);
	std::vector<std::string> getUserCollectionThemeFolders();

	bool themeFolderExists(std::string folder);

	bool includeFileInAutoCollections(FileData* file);

	bool needDoublePress(int presscount);
	int getPressCountInDuration();

	SystemData* mCustomCollectionsBundle;
	SystemData* mRandomCollection;

	static const int DOUBLE_PRESS_DETECTION_DURATION = 1500; // millis
};

std::string getCustomCollectionConfigPath(std::string collectionName);
std::string getCollectionsFolder();
bool systemSort(SystemData* sys1, SystemData* sys2);

#endif // ES_APP_COLLECTION_SYSTEM_MANAGER_H
