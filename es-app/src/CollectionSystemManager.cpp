#include "CollectionSystemManager.h"

#include "guis/GuiInfoPopup.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include "views/gamelist/IGameListView.h"
#include "views/gamelist/ISimpleGameListView.h"
#include "views/ViewController.h"
#include "FileData.h"
#include "FileFilterIndex.h"
#include "Log.h"
#include "Settings.h"
#include "SystemData.h"
#include "ThemeData.h"
#include <pugixml.hpp>
#include <fstream>

/* Handling the getting, initialization, deinitialization, saving and deletion of
 * a CollectionSystemManager Instance */
CollectionSystemManager* CollectionSystemManager::sInstance = NULL;

CollectionSystemManager::CollectionSystemManager(Window* window) : mWindow(window)
{
	CollectionSystemDecl systemDecls[] = {
		//type                  name             long name (display)  default sort (key, order)   theme folder            isCustom
		{ AUTO_ALL_GAMES,       "all",           "all games",         "name, ascending",          "auto-allgames",        false },
		{ AUTO_LAST_PLAYED,     "recent",        "last played",       "last played, descending",  "auto-lastplayed",      false },
		{ AUTO_FAVORITES,       "favorites",     "favorites",         "name, ascending",          "auto-favorites",       false },
		{ AUTO_RANDOM,          RANDOM_COLL_ID,  "random",            "name, ascending",          "auto-random",          false },
		{ CUSTOM_COLLECTION,    CUSTOM_COLL_ID,  "collections",       "name, ascending",          "custom-collections",   true  }
	};

	// create a map
	std::vector<CollectionSystemDecl> tempSystemDecl = std::vector<CollectionSystemDecl>(systemDecls, systemDecls + sizeof(systemDecls) / sizeof(systemDecls[0]));

	for (std::vector<CollectionSystemDecl>::const_iterator it = tempSystemDecl.cbegin(); it != tempSystemDecl.cend(); ++it )
	{
		mCollectionSystemDeclsIndex[(*it).name] = (*it);
	}

	// creating standard environment data
	mCollectionEnvData = new SystemEnvironmentData;
	mCollectionEnvData->mStartPath = "";
	std::vector<std::string> exts;
	mCollectionEnvData->mSearchExtensions = exts;
	mCollectionEnvData->mLaunchCommand = "";
	std::vector<PlatformIds::PlatformId> allPlatformIds;
	allPlatformIds.push_back(PlatformIds::PLATFORM_IGNORE);
	mCollectionEnvData->mPlatformIds = allPlatformIds;

	std::string path = getCollectionsFolder();
	if(!Utils::FileSystem::exists(path))
		Utils::FileSystem::createDirectory(path);

	mIsEditingCustom = false;
	mEditingCollection = "Favorites";
	mEditingCollectionSystemData = NULL;
	mCustomCollectionsBundle = NULL;
	mRandomCollection = NULL;
}

CollectionSystemManager::~CollectionSystemManager()
{
	assert(sInstance == this);
	removeCollectionsFromDisplayedSystems();

	// iterate the map
	for(std::map<std::string, CollectionSystemData>::const_iterator it = mCustomCollectionSystemsData.cbegin() ; it != mCustomCollectionSystemsData.cend() ; it++ )
	{
		if (it->second.isPopulated)
		{
			saveCustomCollection(it->second.system);
		}
		delete it->second.system;
	}
	sInstance = NULL;
}

CollectionSystemManager* CollectionSystemManager::get()
{
	assert(sInstance);
	return sInstance;
}

void CollectionSystemManager::init(Window* window)
{
	assert(!sInstance);
	sInstance = new CollectionSystemManager(window);
}

void CollectionSystemManager::deinit()
{
	if (sInstance)
	{
		delete sInstance;
	}
}

void CollectionSystemManager::saveCustomCollection(SystemData* sys)
{
	std::string name = sys->getName();
	std::unordered_map<std::string, FileData*> games = sys->getRootFolder()->getChildrenByFilename();
	bool found = mCustomCollectionSystemsData.find(name) != mCustomCollectionSystemsData.cend();
	if (found) {
		CollectionSystemData sysData = mCustomCollectionSystemsData.at(name);
		if (sysData.needsSave)
		{
			std::ofstream configFile;
			configFile.open(getCustomCollectionConfigPath(name));
			for(std::unordered_map<std::string, FileData*>::const_iterator iter = games.cbegin(); iter != games.cend(); ++iter)
			{
				std::string path =  iter->first;
				configFile << path << std::endl;
			}
			configFile.close();
		}
	}
	else
	{
		LOG(LogError) << "Couldn't find collection to save! " << name;
	}
}

/* Methods to load all Collections into memory, and handle enabling the active ones */
// loads all Collection Systems
void CollectionSystemManager::loadCollectionSystems(bool async)
{
	initAutoCollectionSystems();
	CollectionSystemDecl decl = mCollectionSystemDeclsIndex[CUSTOM_COLL_ID];
	mCustomCollectionsBundle = createNewCollectionEntry(decl.name, decl, false);
	// we will also load custom systems here
	initCustomCollectionSystems();
	if(Settings::getInstance()->getString("CollectionSystemsAuto") != "" || Settings::getInstance()->getString("CollectionSystemsCustom") != "")
	{
		// Now see which ones are enabled
		loadEnabledListFromSettings();

		// add to the main System Vector, and create Views as needed
		if (!async)
			updateSystemsList();
	}
}

// loads settings
void CollectionSystemManager::loadEnabledListFromSettings()
{
	// we parse the auto collection settings list
	std::vector<std::string> autoSelected = Utils::String::delimitedStringToVector(Settings::getInstance()->getString("CollectionSystemsAuto"), ",", true);

	// iterate the map
	for(std::map<std::string, CollectionSystemData>::iterator it = mAutoCollectionSystemsData.begin() ; it != mAutoCollectionSystemsData.end() ; it++ )
	{
		it->second.isEnabled = (std::find(autoSelected.cbegin(), autoSelected.cend(), it->first) != autoSelected.cend());
	}

	// we parse the custom collection settings list
	std::vector<std::string> customSelected = Utils::String::delimitedStringToVector(Settings::getInstance()->getString("CollectionSystemsCustom"), ",", true);

	// iterate the map
	for(std::map<std::string, CollectionSystemData>::iterator it = mCustomCollectionSystemsData.begin() ; it != mCustomCollectionSystemsData.end() ; it++ )
	{
		it->second.isEnabled = (std::find(customSelected.cbegin(), customSelected.cend(), it->first) != customSelected.cend());
	}
}

// updates enabled system list in System View
void CollectionSystemManager::updateSystemsList()
{
	// remove all Collection Systems
	removeCollectionsFromDisplayedSystems();
	// add custom enabled ones
	addEnabledCollectionsToDisplayedSystems(&mCustomCollectionSystemsData, false);

	if(Settings::getInstance()->getBool("SortAllSystems"))
	{
		// sort custom individual systems with other systems
		std::sort(SystemData::sSystemVector.begin(), SystemData::sSystemVector.end(), systemSort);

		// move RetroPie system to end, before auto collections
		for(auto sysIt = SystemData::sSystemVector.cbegin(); sysIt != SystemData::sSystemVector.cend(); )
		{
			if ((*sysIt)->getName() == "retropie")
			{
				SystemData* retroPieSystem = (*sysIt);
				sysIt = SystemData::sSystemVector.erase(sysIt);
				SystemData::sSystemVector.push_back(retroPieSystem);
				break;
			}
			else
			{
				sysIt++;
			}
		}
	}

	if(mCustomCollectionsBundle->getRootFolder()->getChildren().size() > 0)
	{
		mCustomCollectionsBundle->getRootFolder()->sort(getSortTypeFromString(mCollectionSystemDeclsIndex[CUSTOM_COLL_ID].defaultSort));
		SystemData::sSystemVector.push_back(mCustomCollectionsBundle);
	}

	// add auto enabled ones except random
	addEnabledCollectionsToDisplayedSystems(&mAutoCollectionSystemsData, false);
	// finally, add random
	addEnabledCollectionsToDisplayedSystems(&mAutoCollectionSystemsData, true);

	// create views for collections, before reload
	for(auto sysIt = SystemData::sSystemVector.cbegin(); sysIt != SystemData::sSystemVector.cend(); sysIt++)
	{
		if ((*sysIt)->isCollection())
			ViewController::get()->getGameListView((*sysIt));
	}

	// if we were editing a custom collection, and it's no longer enabled, exit edit mode
	if(mIsEditingCustom && !mEditingCollectionSystemData->isEnabled)
	{
		exitEditMode();
	}
}

/* Methods to manage collection files related to a source FileData */
// updates all collection files related to the source file
void CollectionSystemManager::refreshCollectionSystems(FileData* file)
{
	if (!file->getSystem()->isGameSystem() || file->getType() != GAME)
		return;

	std::map<std::string, CollectionSystemData> allCollections;
	allCollections.insert(mAutoCollectionSystemsData.cbegin(), mAutoCollectionSystemsData.cend());
	allCollections.insert(mCustomCollectionSystemsData.cbegin(), mCustomCollectionSystemsData.cend());

	for(auto sysDataIt = allCollections.cbegin(); sysDataIt != allCollections.cend(); sysDataIt++)
	{
		updateCollectionSystem(file, sysDataIt->second);
	}
}

void CollectionSystemManager::updateCollectionSystem(FileData* file, CollectionSystemData sysData)
{
	if (sysData.isPopulated)
	{
		// collection files use the full path as key, to avoid clashes
		std::string key = file->getFullPath();

		SystemData* curSys = sysData.system;
		const std::unordered_map<std::string, FileData*>& children = curSys->getRootFolder()->getChildrenByFilename();
		bool found = children.find(key) != children.cend();
		FileData* rootFolder = curSys->getRootFolder();
		FileFilterIndex* fileIndex = curSys->getIndex();
		std::string name = curSys->getName();

		if (found) {
			// if we found it, we need to update it
			FileData* collectionEntry = children.at(key);
			// remove from index, so we can re-index metadata after refreshing
			fileIndex->removeFromIndex(collectionEntry);
			collectionEntry->refreshMetadata();
			// found and we are removing
			if (name == "favorites" && file->metadata.get("favorite") == "false") {
				// need to check if still marked as favorite, if not remove
				ViewController::get()->getGameListView(curSys).get()->remove(collectionEntry, false, true);
			}
			else
			{
				// re-index with new metadata
				fileIndex->addToIndex(collectionEntry);
				ViewController::get()->onFileChanged(collectionEntry, FILE_METADATA_CHANGED);
			}
		}
		else
		{
			// we didn't find it here - we need to check if we should add it
			if (name == "recent" && file->metadata.get("playcount") > "0" && includeFileInAutoCollections(file) ||
				name == "favorites" && file->metadata.get("favorite") == "true") {
				CollectionFileData* newGame = new CollectionFileData(file, curSys);
				rootFolder->addChild(newGame);
				fileIndex->addToIndex(newGame);
				ViewController::get()->onFileChanged(file, FILE_METADATA_CHANGED);
				ViewController::get()->getGameListView(curSys)->onFileChanged(newGame, FILE_METADATA_CHANGED);
			}
		}
		rootFolder->sort(getSortTypeFromString(mCollectionSystemDeclsIndex[name].defaultSort));
		if (name == "recent")
		{
			trimCollectionCount(rootFolder, LAST_PLAYED_MAX, false);
			ViewController::get()->onFileChanged(rootFolder, FILE_METADATA_CHANGED);
		}
		else
			ViewController::get()->onFileChanged(rootFolder, FILE_SORTED);
	}
}

void CollectionSystemManager::trimCollectionCount(FileData* rootFolder, int limit, bool shuffle)
{
	SystemData* curSys = rootFolder->getSystem();
	while ((int)rootFolder->getChildrenListToDisplay().size() > limit)
	{
		std::vector<FileData*> games = rootFolder->getFilesRecursive(GAME, true);
		if (shuffle)
			std::shuffle(games.begin(), games.end(), SystemData::sURNG);

		CollectionFileData* gameToRemove = (CollectionFileData*)games.back();
		ViewController::get()->getGameListView(curSys).get()->remove(gameToRemove, false, false);
	}
	ViewController::get()->onFileChanged(rootFolder, FILE_REMOVED);
}

// deletes all collection files from collection systems related to the source file
void CollectionSystemManager::deleteCollectionFiles(FileData* file)
{
	// collection files use the full path as key, to avoid clashes
	std::string key = file->getFullPath();
	// find games in collection systems
	std::map<std::string, CollectionSystemData> allCollections;
	allCollections.insert(mAutoCollectionSystemsData.cbegin(), mAutoCollectionSystemsData.cend());
	allCollections.insert(mCustomCollectionSystemsData.cbegin(), mCustomCollectionSystemsData.cend());

	for(auto sysDataIt = allCollections.begin(); sysDataIt != allCollections.end(); sysDataIt++)
	{
		if (sysDataIt->second.isPopulated)
		{
			const std::unordered_map<std::string, FileData*>& children = (sysDataIt->second.system)->getRootFolder()->getChildrenByFilename();

			bool found = children.find(key) != children.cend();
			if (found) {
				sysDataIt->second.needsSave = true;
				FileData* collectionEntry = children.at(key);
				SystemData* systemViewToUpdate = getSystemToView(sysDataIt->second.system);
				ViewController::get()->getGameListView(systemViewToUpdate).get()->remove(collectionEntry, false, true);
			}
		}
	}
}

// returns whether the current theme is compatible with Automatic or Custom Collections
bool CollectionSystemManager::isThemeGenericCollectionCompatible(bool genericCustomCollections)
{
	std::vector<std::string> cfgSys = getCollectionThemeFolders(genericCustomCollections);
	for(auto sysIt = cfgSys.cbegin(); sysIt != cfgSys.cend(); sysIt++)
	{
		if(!themeFolderExists(*sysIt))
			return false;
	}
	return true;
}

bool CollectionSystemManager::isThemeCustomCollectionCompatible(std::vector<std::string> stringVector)
{
	if (isThemeGenericCollectionCompatible(true))
		return true;

	// get theme path
	auto themeSets = ThemeData::getThemeSets();
	auto set = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
	if(set != themeSets.cend())
	{
		std::string defaultThemeFilePath = set->second.path + "/theme.xml";
		if (Utils::FileSystem::exists(defaultThemeFilePath))
		{
			return true;
		}
	}

	for(auto sysIt = stringVector.cbegin(); sysIt != stringVector.cend(); sysIt++)
	{
		if(!themeFolderExists(*sysIt))
			return false;
	}
	return true;
}

std::string CollectionSystemManager::getValidNewCollectionName(std::string inName, int index)
{
	std::string name = inName;

	if(index == 0)
	{
		size_t remove = std::string::npos;

		// get valid name
		while((remove = name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-[]() ")) != std::string::npos)
		{
			name.erase(remove, 1);
		}
	}
	else
	{
		name += " (" + std::to_string(index) + ")";
	}

	if(name == "")
	{
		name = "New Collection";
	}

	if(name != inName)
	{
		LOG(LogInfo) << "Had to change name, from: " << inName << " to: " << name;
	}

	// get used systems in es_systems.cfg
	std::vector<std::string> systemsInUse = getSystemsFromConfig();
	// get folders assigned to custom collections
	std::vector<std::string> autoSys = getCollectionThemeFolders(false);
	// get folder assigned to custom collections
	std::vector<std::string> customSys = getCollectionThemeFolders(true);
	// get folders assigned to user collections
	std::vector<std::string> userSys = getUserCollectionThemeFolders();
	// add them all to the list of systems in use
	systemsInUse.insert(systemsInUse.cend(), autoSys.cbegin(), autoSys.cend());
	systemsInUse.insert(systemsInUse.cend(), customSys.cbegin(), customSys.cend());
	systemsInUse.insert(systemsInUse.cend(), userSys.cbegin(), userSys.cend());
	for(auto sysIt = systemsInUse.cbegin(); sysIt != systemsInUse.cend(); sysIt++)
	{
		if (*sysIt == name)
		{
			if(index > 0) {
				name = name.substr(0, name.size()-4);
			}
			return getValidNewCollectionName(name, index+1);
		}
	}
	// if it matches one of the custom collections reserved names
	if (mCollectionSystemDeclsIndex.find(name) != mCollectionSystemDeclsIndex.cend())
		return getValidNewCollectionName(name, index+1);
	return name;
}

void CollectionSystemManager::setEditMode(std::string collectionName, bool quiet)
{
	if (mCustomCollectionSystemsData.find(collectionName) == mCustomCollectionSystemsData.cend())
	{
		LOG(LogError) << "Tried to edit a non-existing collection: " << collectionName;
		return;
	}
	mIsEditingCustom = true;
	mEditingCollection = collectionName;

	CollectionSystemData* sysData = &(mCustomCollectionSystemsData.at(mEditingCollection));
	if (!sysData->isPopulated)
	{
		populateCustomCollection(sysData);
	}
	// if it's bundled, this needs to be the bundle system
	mEditingCollectionSystemData = sysData;

	if (!quiet) {
		GuiInfoPopup* s = new GuiInfoPopup(mWindow, "Editing the '" + Utils::String::toUpper(collectionName) + "' Collection. Add/remove games with Y.", 10000);
		mWindow->setInfoPopup(s);
	}
}

void CollectionSystemManager::exitEditMode(bool quiet)
{
	if (!quiet) {
		GuiInfoPopup* s = new GuiInfoPopup(mWindow, "Finished editing the '" + mEditingCollection + "' Collection.", 4000);
		mWindow->setInfoPopup(s);
	}
	if (mIsEditingCustom) {
		mIsEditingCustom = false;
		mEditingCollection = "Favorites";

		mEditingCollectionSystemData->system->onMetaDataSavePoint();
	}
}

int CollectionSystemManager::getPressCountInDuration() {
	Uint32 now = SDL_GetTicks();
	if (now - mFirstPressMs < DOUBLE_PRESS_DETECTION_DURATION) {
		return 2;
	} else {
		mFirstPressMs = now;
		return 1;
	}
}

// adds or removes a game from a specific collection
bool CollectionSystemManager::toggleGameInCollection(FileData* file)
{
	if (file->getType() == GAME)
	{
		GuiInfoPopup* s;
		bool adding = true;
		std::string name = file->getName();
		std::string sysName = mEditingCollection;
		if (mIsEditingCustom)
		{
			SystemData* sysData = mEditingCollectionSystemData->system;
			mEditingCollectionSystemData->needsSave = true;
			if (!mEditingCollectionSystemData->isPopulated)
			{
				populateCustomCollection(mEditingCollectionSystemData);
			}
			std::string key = file->getFullPath();
			FileData* rootFolder = sysData->getRootFolder();
			const std::unordered_map<std::string, FileData*>& children = rootFolder->getChildrenByFilename();
			bool found = children.find(key) != children.cend();
			FileFilterIndex* fileIndex = sysData->getIndex();
			std::string name = sysData->getName();

			SystemData* systemViewToUpdate = getSystemToView(sysData);

			if (found) {
				if (needDoublePress(getPressCountInDuration())) {
					return true;
				}
				adding = false;
				// if we found it, we need to remove it
				FileData* collectionEntry = children.at(key);
				// remove from index
				fileIndex->removeFromIndex(collectionEntry);
				// remove from bundle index as well, if needed
				if(systemViewToUpdate != sysData)
				{
					systemViewToUpdate->getIndex()->removeFromIndex(collectionEntry);
				}
				ViewController::get()->getGameListView(systemViewToUpdate).get()->remove(collectionEntry, false, true);
			}
			else
			{
				// we didn't find it here, we should add it
				CollectionFileData* newGame = new CollectionFileData(file, sysData);
				rootFolder->addChild(newGame);
				fileIndex->addToIndex(newGame);
				// this is the biggest performance bottleneck for this process.
				// this code has been here for 7 years, since this feature was added.
				// I might have been playing it safe back then, but it feels unnecessary, especially given following onFileChanged to sort
				// Commenting this out for now.
				//ViewController::get()->getGameListView(systemViewToUpdate)->onFileChanged(newGame, FILE_METADATA_CHANGED);
				rootFolder->sort(getSortTypeFromString(mEditingCollectionSystemData->decl.defaultSort));
				ViewController::get()->onFileChanged(systemViewToUpdate->getRootFolder(), FILE_SORTED);
				// add to bundle index as well, if needed
				if(systemViewToUpdate != sysData)
				{
					systemViewToUpdate->getIndex()->addToIndex(newGame);
				}
			}
			sysData->setShuffledCacheDirty();
			updateCollectionFolderMetadata(sysData);
		}
		else
		{
			file->getSourceFileData()->getSystem()->getIndex()->removeFromIndex(file);
			MetaDataList* md = &file->getSourceFileData()->metadata;
			std::string value = md->get("favorite");
			if (value == "false")
			{
				md->set("favorite", "true");
			}
			else
			{
				if (needDoublePress(getPressCountInDuration())) {
					return true;
				}
				adding = false;
				md->set("favorite", "false");
			}
			file->getSourceFileData()->getSystem()->getIndex()->addToIndex(file);

			file->getSourceFileData()->getSystem()->onMetaDataSavePoint();

			refreshCollectionSystems(file->getSourceFileData());
		}
		if (adding)
		{
			s = new GuiInfoPopup(mWindow, "Added '" + Utils::String::removeParenthesis(name) + "' to '" + Utils::String::toUpper(sysName) + "'", 4000);
		}
		else
		{
			s = new GuiInfoPopup(mWindow, "Removed '" + Utils::String::removeParenthesis(name) + "' from '" + Utils::String::toUpper(sysName) + "'", 4000);
		}

		mWindow->setInfoPopup(s);
		return true;
	}
	return false;
}


SystemData* CollectionSystemManager::getSystemToView(SystemData* sys)
{
	SystemData* systemToView = sys;
	FileData* rootFolder = sys->getRootFolder();

	FileData* bundleRootFolder = mCustomCollectionsBundle->getRootFolder();
	const std::unordered_map<std::string, FileData*>& bundleChildren = bundleRootFolder->getChildrenByFilename();

	// is the rootFolder bundled in the "My Collections" system?
	bool sysFoundInBundle = bundleChildren.find(rootFolder->getKey()) != bundleChildren.cend();

	if (sysFoundInBundle && sys->isCollection())
	{
		systemToView = mCustomCollectionsBundle;
	}
	return systemToView;
}

void CollectionSystemManager::recreateCollection(SystemData* sysData)
{
	CollectionSystemData* colSysData;
	if (mAutoCollectionSystemsData.find(sysData->getName()) != mAutoCollectionSystemsData.end())
	{
		// it's an auto collection
		colSysData = &mAutoCollectionSystemsData[sysData->getName()];
	}
	else if (mCustomCollectionSystemsData.find(sysData->getName()) != mCustomCollectionSystemsData.end())
	{
		// it's a custom collection
		colSysData = &mCustomCollectionSystemsData[sysData->getName()];
	}
	else
	{
		LOG(LogDebug) << "Couldn't find collection to recreate in either custom or auto collections: " << sysData->getName();
		return;
	}

	CollectionSystemDecl sysDecl = colSysData->decl;
	FileData* rootFolder = sysData->getRootFolder();
	FileFilterIndex* index = sysData->getIndex();
	const std::unordered_map<std::string, FileData*>& children = rootFolder->getChildrenByFilename();

	sysData->getIndex()->resetIndex();
	std::string name = sysData->getName();

	SystemData* systemViewToUpdate = getSystemToView(sysData);

	// while there are games there, remove them from the view and system
	while(rootFolder->getChildrenByFilename().size() > 0)
		ViewController::get()->getGameListView(systemViewToUpdate).get()->remove(rootFolder->getChildrenByFilename().begin()->second, false, false);

	colSysData->isPopulated = false;
	if (sysDecl.isCustom)
		populateCustomCollection(colSysData);
	else
		populateAutoCollection(colSysData);

	rootFolder->sort(getSortTypeFromString(colSysData->decl.defaultSort));
	ViewController::get()->onFileChanged(systemViewToUpdate->getRootFolder(), FILE_SORTED);

	// Workaround to force video to play
	FileData* cursor = ViewController::get()->getGameListView(systemViewToUpdate)->getCursor();
	ViewController::get()->getGameListView(systemViewToUpdate)->setCursor(cursor, true);


}

/* Handles loading a collection system, creating an empty one, and populating on demand */
// loads Automatic Collection systems (All, Favorites, Last Played, Random)
void CollectionSystemManager::initAutoCollectionSystems()
{
	for(std::map<std::string, CollectionSystemDecl>::const_iterator it = mCollectionSystemDeclsIndex.cbegin() ; it != mCollectionSystemDeclsIndex.cend() ; it++ )
	{
		CollectionSystemDecl sysDecl = it->second;
		if (!sysDecl.isCustom)
		{
			SystemData* newCol = createNewCollectionEntry(sysDecl.name, sysDecl);
			if (sysDecl.type == AUTO_RANDOM)
				mRandomCollection = newCol;
		}
	}
}

// this may come in handy if at any point in time in the future we want to
// automatically generate metadata for a folder
void CollectionSystemManager::updateCollectionFolderMetadata(SystemData* sys)
{
	FileData* rootFolder = sys->getRootFolder();

	std::string desc = "This collection is empty.";
	std::string rating = "0";
	std::string players = "1";
	std::string releasedate = "N/A";
	std::string developer = "None";
	std::string genre = "None";
	std::string video = "";
	std::string thumbnail = "";
	std::string image = "";

	std::unordered_map<std::string, FileData*> games = rootFolder->getChildrenByFilename();

	if(games.size() > 0)
	{
		std::string games_list = "";
		int games_counter = 0;
		for(std::unordered_map<std::string, FileData*>::const_iterator iter = games.cbegin(); iter != games.cend(); ++iter)
		{
			games_counter++;
			FileData* file = iter->second;

			std::string new_rating = file->metadata.get("rating");
			std::string new_releasedate = file->metadata.get("releasedate");
			std::string new_developer = file->metadata.get("developer");
			std::string new_genre = file->metadata.get("genre");
			std::string new_players = file->metadata.get("players");

			rating = (new_rating > rating ? (new_rating != "" ? new_rating : rating) : rating);
			players = (new_players > players ? (new_players != "" ? new_players : players) : players);
			releasedate = (new_releasedate < releasedate ? (new_releasedate != "" ? new_releasedate : releasedate) : releasedate);
			developer = (developer == "None" ? new_developer : (new_developer != developer ? "Various" : new_developer));
			genre = (genre == "None" ? new_genre : (new_genre != genre ? "Various" : new_genre));

			switch(games_counter)
			{
				case 2:
				case 3:
					games_list += ", ";
				case 1:
					games_list += "'" + file->getName() + "'";
					break;
				case 4:
					games_list += " among other titles.";
			}
		}

		desc = "This collection contains " + std::to_string(games_counter) + " game"
				+ (games_counter == 1 ? "" : "s") + ", including " + games_list;

		FileData* randomGame = sys->getRandomGame();

		video = randomGame->getVideoPath();
		thumbnail = randomGame->getThumbnailPath();
		image = randomGame->getImagePath();
	}


	rootFolder->metadata.set("desc", desc);
	rootFolder->metadata.set("rating", rating);
	rootFolder->metadata.set("players", players);
	rootFolder->metadata.set("genre", genre);
	rootFolder->metadata.set("releasedate", releasedate);
	rootFolder->metadata.set("developer", developer);
	rootFolder->metadata.set("video", video);
	rootFolder->metadata.set("thumbnail", thumbnail);
	rootFolder->metadata.set("image", image);
}

void CollectionSystemManager::initCustomCollectionSystems()
{
	std::vector<std::string> systems = getCollectionsFromConfigFolder();
	for (auto nameIt = systems.cbegin(); nameIt != systems.cend(); nameIt++)
	{
		addNewCustomCollection(*nameIt);
	}
}

SystemData* CollectionSystemManager::getAllGamesCollection()
{
	CollectionSystemData* allSysData = &mAutoCollectionSystemsData["all"];
	if (!allSysData->isPopulated)
	{
		populateAutoCollection(allSysData);
	}
	return allSysData->system;
}

SystemData* CollectionSystemManager::addNewCustomCollection(std::string name)
{
	CollectionSystemDecl decl = mCollectionSystemDeclsIndex[CUSTOM_COLL_ID];
	decl.themeFolder = name;
	decl.name = name;
	decl.longName = name;
	return createNewCollectionEntry(name, decl);
}

// creates a new, empty Collection system, based on the name and declaration
SystemData* CollectionSystemManager::createNewCollectionEntry(std::string name, CollectionSystemDecl sysDecl, bool index)
{
	SystemData* newSys = new SystemData(name, sysDecl.longName, mCollectionEnvData, sysDecl.themeFolder, true);

	CollectionSystemData newCollectionData;
	newCollectionData.system = newSys;
	newCollectionData.decl = sysDecl;
	newCollectionData.isEnabled = false;
	newCollectionData.isPopulated = false;
	newCollectionData.needsSave = false;

	if (index)
	{
		if (!sysDecl.isCustom)
		{
			mAutoCollectionSystemsData[name] = newCollectionData;
		}
		else
		{
			mCustomCollectionSystemsData[name] = newCollectionData;
		}
	}

	return newSys;
}

void CollectionSystemManager::addRandomGames(SystemData* newSys, SystemData* sourceSystem, FileData* rootFolder,
	FileFilterIndex* index, std::map<std::string, std::map<std::string, int>> mapsForRandomColl, int defaultValue)
{

	int gamesForSourceSystem = defaultValue;
	for (auto& m : mapsForRandomColl)
	{
		// m.first unused
		std::map<std::string, int> collMap = m.second;
		if (collMap.find(sourceSystem->getFullName()) != collMap.end())
		{
			int maxForSys = collMap[sourceSystem->getFullName()];
			// we won't add more than the max and less than 0
			gamesForSourceSystem = Math::max(Math::min(RANDOM_SYSTEM_MAX, maxForSys), 0);
			break;
		}
	}

	// load exclusion collection
	std::unordered_map<std::string,FileData*> exclusionMap;
	std::string exclusionCollection = Settings::getInstance()->getString("RandomCollectionExclusionCollection");
	auto sysDataIt = mCustomCollectionSystemsData.find(exclusionCollection);

	if (!exclusionCollection.empty() && sysDataIt != mCustomCollectionSystemsData.end()) {
		if (!sysDataIt->second.isPopulated)
		{
			populateCustomCollection(&(sysDataIt->second));
		}

		exclusionMap = mCustomCollectionSystemsData[exclusionCollection].system->getRootFolder()->getChildrenByFilename();

	}

	// we do this to avoid trying to add more games than there are in the system
	gamesForSourceSystem = Math::min(gamesForSourceSystem, sourceSystem->getRootFolder()->getFilesRecursive(GAME).size());

	int startCount = rootFolder->getFilesRecursive(GAME).size();
	int endCount = startCount + gamesForSourceSystem;
	int retryCount = 10;

	for (int iterCount = startCount; iterCount < endCount;)
	{
		FileData* randomGame = sourceSystem->getRandomGame()->getSourceFileData();
		CollectionFileData* newGame = NULL;

		if(exclusionMap.find(randomGame->getFullPath()) == exclusionMap.end())
		{
			// Not in the exclusion collection
			newGame = new CollectionFileData(randomGame, newSys);
			rootFolder->addChild(newGame);
			index->addToIndex(newGame);
		}

		if (rootFolder->getFilesRecursive(GAME).size() > iterCount)
		{
			// added game, proceed
			iterCount++;
			retryCount = 10;
		}
		else
		{
			// the game already exists in the collection, let's try again
			LOG(LogDebug) << "Clash: " << randomGame->getName() << " already exists or in exclusion list. Deleting and trying again";
			delete newGame;
			retryCount--;
			if (retryCount == 0)
			{
				// we give up. Either we were very unlucky, or all the games in this system are already there.
				LOG(LogDebug) << "Giving up retrying: cannot add this game. Deleting and moving on.";
				return;
			}
		}
	}
}

void CollectionSystemManager::populateRandomCollectionFromCollections(std::map<std::string, std::map<std::string, int>> mapsForRandomColl)
{
	CollectionSystemData* sysData = &mAutoCollectionSystemsData[RANDOM_COLL_ID];
	SystemData* newSys = sysData->system;
	CollectionSystemDecl sysDecl = sysData->decl;
	FileData* rootFolder = newSys->getRootFolder();
	FileFilterIndex* index = newSys->getIndex();

	// iterate the auto collections map
	for(auto &c : mAutoCollectionSystemsData)
	{
		CollectionSystemData csd = c.second;
		// we can't add games from the random collection to the random collection
		if (csd.decl.type != AUTO_RANDOM)
		{
			// collections might not be populated
			if (!csd.isPopulated)
				populateAutoCollection(&csd);

			if (csd.isPopulated)
				addRandomGames(newSys, csd.system, rootFolder, index, mapsForRandomColl, DEFAULT_RANDOM_COLLECTIONS_GAMES);
		}
	}

	// iterate the custom collections map
	for(auto &c : mCustomCollectionSystemsData)
	{
		CollectionSystemData csd = c.second;
		// collections might not be populated
		if (!csd.isPopulated)
			populateCustomCollection(&csd);

		if (csd.isPopulated)
			addRandomGames(newSys, csd.system, rootFolder, index, mapsForRandomColl, DEFAULT_RANDOM_COLLECTIONS_GAMES);
	}
}

// populates an Automatic Collection System
void CollectionSystemManager::populateAutoCollection(CollectionSystemData* sysData)
{
	SystemData* newSys = sysData->system;
	CollectionSystemDecl sysDecl = sysData->decl;
	FileData* rootFolder = newSys->getRootFolder();
	FileFilterIndex* index = newSys->getIndex();

	std::map<std::string, std::map<std::string, int>> mapsForRandomColl;
	if (sysDecl.type == AUTO_RANDOM)
	{
		// user may have defined a custom collection with the same name as a system name, thus keeping maps in another map
		std::map<std::string, int> randomSystems = Settings::getInstance()->getMap("RandomCollectionSystems");
		mapsForRandomColl["RandomCollectionSystems"] = randomSystems;
		std::map<std::string, int> randomAutoColl = Settings::getInstance()->getMap("RandomCollectionSystemsAuto");
		mapsForRandomColl["RandomCollectionSystemsAuto"] = randomAutoColl;
		std::map<std::string, int> randomCustColl = Settings::getInstance()->getMap("RandomCollectionSystemsCustom");
		mapsForRandomColl["RandomCollectionSystemsCustom"] = randomCustColl;
	}
	// Only iterate through game systems, not collections yet
	for(auto sysIt = SystemData::sSystemVector.cbegin(); sysIt != SystemData::sSystemVector.cend(); sysIt++)
	{
		// we won't iterate all collections
		if ((*sysIt)->isGameSystem() && !(*sysIt)->isCollection())
		{
			if (sysDecl.type == AUTO_RANDOM)
			{
				addRandomGames(newSys, *sysIt, rootFolder, index, mapsForRandomColl, DEFAULT_RANDOM_SYSTEM_GAMES);
			}
			else
			{
				std::vector<FileData*> files = (*sysIt)->getRootFolder()->getFilesRecursive(GAME);

				for(auto gameIt = files.cbegin(); gameIt != files.cend(); gameIt++)
				{
					bool include = includeFileInAutoCollections(*gameIt);
					switch(sysDecl.type) {
						case AUTO_LAST_PLAYED:
							include = include && (*gameIt)->metadata.get("playcount") > "0";
							break;
						case AUTO_FAVORITES:
							// we may still want to add files we don't want in auto collections in "favorites"
							include = (*gameIt)->metadata.get("favorite") == "true";
							break;
						case AUTO_ALL_GAMES:
							break;
						default:
							// No-op to prevent compiler warnings
							// Getting here means that the file is not part of a pre-defined collection.
							include = false;
							break;
					}

					if (include)
					{
						CollectionFileData* newGame = new CollectionFileData(*gameIt, newSys);
						rootFolder->addChild(newGame);
						index->addToIndex(newGame);
					}
				}
			}
		}
	}

	// here we finish populating the Random collection based on other Collections
	if (sysDecl.type == AUTO_RANDOM)
		populateRandomCollectionFromCollections(mapsForRandomColl);

	// sort before optional trimming, if collection is displayed
	if (sysData->isEnabled)
		rootFolder->sort(getSortTypeFromString(sysDecl.defaultSort));

	if (sysData->isEnabled && (sysDecl.type == AUTO_LAST_PLAYED || sysDecl.type == AUTO_RANDOM))
	{
		int trimValue = LAST_PLAYED_MAX;
		if (sysDecl.type == AUTO_RANDOM)
			trimValue = Settings::getInstance()->getInt("RandomCollectionMaxGames");
		if (trimValue > 0)
			trimCollectionCount(rootFolder, trimValue, sysDecl.type == AUTO_RANDOM);
	}

	sysData->isPopulated = true;
}

// populates a Custom Collection System
void CollectionSystemManager::populateCustomCollection(CollectionSystemData* sysData)
{
	SystemData* newSys = sysData->system;
	CollectionSystemDecl sysDecl = sysData->decl;
	std::string path = getCustomCollectionConfigPath(newSys->getName());

	if(!Utils::FileSystem::exists(path))
	{
		LOG(LogInfo) << "Couldn't find custom collection config file at " << path;
		return;
	}
	LOG(LogInfo) << "Loading custom collection config file at " << path;

	FileData* rootFolder = newSys->getRootFolder();
	FileFilterIndex* index = newSys->getIndex();

	// get Configuration for this Custom System
	std::ifstream input(path);

	// get all files map
	std::unordered_map<std::string,FileData*> allFilesMap = getAllGamesCollection()->getRootFolder()->getChildrenByFilename();

	// iterate list of files in config file
	for(std::string gameKey; getline(input, gameKey); )
	{
		std::unordered_map<std::string,FileData*>::const_iterator it = allFilesMap.find(gameKey);
		if (it != allFilesMap.cend())
		{
			CollectionFileData* newGame = new CollectionFileData(it->second, newSys);
			rootFolder->addChild(newGame);
			index->addToIndex(newGame);
		}
		else
		{
			LOG(LogInfo) << "Couldn't find game referenced at '" << gameKey << "' for system config '" << path << "'";
		}
	}
	rootFolder->sort(getSortTypeFromString(sysDecl.defaultSort));
	updateCollectionFolderMetadata(newSys);
	sysData->isPopulated = true;
}

/* Handle System View removal and insertion of Collections */
void CollectionSystemManager::removeCollectionsFromDisplayedSystems()
{
	// remove all Collection Systems
	for(auto sysIt = SystemData::sSystemVector.cbegin(); sysIt != SystemData::sSystemVector.cend(); )
	{
		if ((*sysIt)->isCollection())
		{
			sysIt = SystemData::sSystemVector.erase(sysIt);
		}
		else
		{
			sysIt++;
		}
	}

	// remove all custom collections in bundle
	// this should not delete the objects from memory!
	if (mCustomCollectionsBundle)
	{
		FileData* customRoot = mCustomCollectionsBundle->getRootFolder();
		std::vector<FileData*> mChildren = customRoot->getChildren();
		for(auto it = mChildren.cbegin(); it != mChildren.cend(); it++)
		{
			customRoot->removeChild(*it);
		}
		// clear index
		mCustomCollectionsBundle->getIndex()->resetIndex();
		// remove view so it's re-created as needed
		ViewController::get()->removeGameListView(mCustomCollectionsBundle);
	}
}

// The "random" collection relies on all other collections to have been initialized, so we defer its processing
void CollectionSystemManager::addEnabledCollectionsToDisplayedSystems(std::map<std::string, CollectionSystemData>* colSystemData, bool processRandom)
{
	// add auto enabled ones
	for(std::map<std::string, CollectionSystemData>::iterator it = colSystemData->begin() ; it != colSystemData->end() ; it++ )
	{

		if ((!processRandom && it->second.decl.type != AUTO_RANDOM) || (processRandom && it->second.decl.type == AUTO_RANDOM))
		{
			if(it->second.isEnabled)
			{
				// check if populated, otherwise populate
				if (!it->second.isPopulated)
				{
					if(it->second.decl.isCustom)
						populateCustomCollection(&(it->second));
					else
						populateAutoCollection(&(it->second));
				}

				// check if it has its own view
				if(!it->second.decl.isCustom || themeFolderExists(it->first) || !Settings::getInstance()->getBool("UseCustomCollectionsSystem"))
				{
					// exists theme folder, or we chose not to bundle it under the custom-collections system
					// so we need to create a view
					SystemData::sSystemVector.push_back(it->second.system);
				}
				else
				{
					FileData* newSysRootFolder = it->second.system->getRootFolder();
					mCustomCollectionsBundle->getRootFolder()->addChild(newSysRootFolder);
					mCustomCollectionsBundle->getIndex()->importIndex(it->second.system->getIndex());
				}
			}
		}
	}
}

/* Auxiliary methods to get available custom collection possibilities */
std::vector<std::string> CollectionSystemManager::getSystemsFromConfig()
{
	std::vector<std::string> systems;
	std::string path = SystemData::getConfigPath(false);

	if(!Utils::FileSystem::exists(path))
	{
		return systems;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());

	if(!res)
	{
		return systems;
	}

	//actually read the file
	pugi::xml_node systemList = doc.child("systemList");

	if(!systemList)
	{
		return systems;
	}

	for(pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		// theme folder
		std::string themeFolder = system.child("theme").text().get();
		systems.push_back(themeFolder);
	}
	std::sort(systems.begin(), systems.end());
	return systems;
}

// gets all folders from the current theme path
std::vector<std::string> CollectionSystemManager::getSystemsFromTheme()
{
	std::vector<std::string> systems;

	auto themeSets = ThemeData::getThemeSets();
	if(themeSets.empty())
	{
		// no theme sets available
		return systems;
	}

	std::map<std::string, ThemeSet>::const_iterator set = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
	if(set == themeSets.cend())
	{
		// currently selected theme set is missing, so just pick the first available set
		set = themeSets.cbegin();
		Settings::getInstance()->setString("ThemeSet", set->first);
	}

	std::string themePath = set->second.path;

	if (Utils::FileSystem::exists(themePath))
	{
		Utils::FileSystem::stringList dirContent = Utils::FileSystem::getDirContent(themePath);

		for (Utils::FileSystem::stringList::const_iterator it = dirContent.cbegin(); it != dirContent.cend(); ++it)
		{
			if (Utils::FileSystem::isDirectory(*it))
			{
				//... here you have a directory
				std::string folder = *it;
				folder = folder.substr(themePath.size()+1);

				if(Utils::FileSystem::exists(set->second.getThemePath(folder)))
				{
					systems.push_back(folder);
				}
			}
		}
	}
	std::sort(systems.begin(), systems.end());
	return systems;
}

// returns the unused folders from current theme path
std::vector<std::string> CollectionSystemManager::getUnusedSystemsFromTheme()
{
	// get used systems in es_systems.cfg
	std::vector<std::string> systemsInUse = getSystemsFromConfig();
	// get available folders in theme
	std::vector<std::string> themeSys = getSystemsFromTheme();
	// get folders assigned to custom collections
	std::vector<std::string> autoSys = getCollectionThemeFolders(false);
	// get folder assigned to custom collections
	std::vector<std::string> customSys = getCollectionThemeFolders(true);
	// get folders assigned to user collections
	std::vector<std::string> userSys = getUserCollectionThemeFolders();
	// add them all to the list of systems in use
	systemsInUse.insert(systemsInUse.cend(), autoSys.cbegin(), autoSys.cend());
	systemsInUse.insert(systemsInUse.cend(), customSys.cbegin(), customSys.cend());
	systemsInUse.insert(systemsInUse.cend(), userSys.cbegin(), userSys.cend());

	for(auto sysIt = themeSys.cbegin(); sysIt != themeSys.cend(); )
	{
		if (std::find(systemsInUse.cbegin(), systemsInUse.cend(), *sysIt) != systemsInUse.cend())
		{
			sysIt = themeSys.erase(sysIt);
		}
		else
		{
			sysIt++;
		}
	}
	return themeSys;
}

// returns which collection config files exist in the user folder
std::vector<std::string> CollectionSystemManager::getCollectionsFromConfigFolder()
{
	std::vector<std::string> systems;
	std::string configPath = getCollectionsFolder();

	if (Utils::FileSystem::exists(configPath))
	{
		Utils::FileSystem::stringList dirContent = Utils::FileSystem::getDirContent(configPath);
		for (Utils::FileSystem::stringList::const_iterator it = dirContent.cbegin(); it != dirContent.cend(); ++it)
		{
			if (Utils::FileSystem::isRegularFile(*it))
			{
				// it's a file
				std::string filename = Utils::FileSystem::getFileName(*it);

				// need to confirm filename matches config format
				if (filename != "custom-.cfg" && Utils::String::startsWith(filename, "custom-") && Utils::String::endsWith(filename, ".cfg"))
				{
					filename = filename.substr(7, filename.size()-11);
					systems.push_back(filename);
				}
				else
				{
					LOG(LogInfo) << "Found non-collection config file in collections folder: " << filename;
				}
			}
		}
	}
	return systems;
}

// returns the theme folders for Automatic Collections (All, Favorites, Last Played) or generic Custom Collections folder
std::vector<std::string> CollectionSystemManager::getCollectionThemeFolders(bool custom)
{
	std::vector<std::string> systems;
	for(std::map<std::string, CollectionSystemDecl>::const_iterator it = mCollectionSystemDeclsIndex.cbegin() ; it != mCollectionSystemDeclsIndex.cend() ; it++ )
	{
		CollectionSystemDecl sysDecl = it->second;
		if (sysDecl.isCustom == custom)
		{
			systems.push_back(sysDecl.themeFolder);
		}
	}
	return systems;
}

// returns the theme folders in use for the user-defined Custom Collections
std::vector<std::string> CollectionSystemManager::getUserCollectionThemeFolders()
{
	std::vector<std::string> systems;
	for(std::map<std::string, CollectionSystemData>::const_iterator it = mCustomCollectionSystemsData.cbegin() ; it != mCustomCollectionSystemsData.cend() ; it++ )
	{
		systems.push_back(it->second.decl.themeFolder);
	}
	return systems;
}

// returns whether a specific folder exists in the theme
bool CollectionSystemManager::themeFolderExists(std::string folder)
{
	std::vector<std::string> themeSys = getSystemsFromTheme();
	return std::find(themeSys.cbegin(), themeSys.cend(), folder) != themeSys.cend();
}

bool CollectionSystemManager::includeFileInAutoCollections(FileData* file)
{
	// we exclude non-game files from collections (i.e. "kodi", entries from non-game systems)
	// if/when there are more in the future, maybe this can be a more complex method, with a proper list
	// but for now a simple string comparison is more performant
	return file->getName() != "kodi" && file->getSystem()->isGameSystem();
}


bool CollectionSystemManager::needDoublePress(int presscount) {
	if (Settings::getInstance()->getBool("DoublePressRemovesFromFavs") && presscount < 2)
	{
		GuiInfoPopup* toast = new GuiInfoPopup(mWindow, "Press again to remove from '" + Utils::String::toUpper(mEditingCollection)
		+ "'", DOUBLE_PRESS_DETECTION_DURATION, 100, 200);
		mWindow->setInfoPopup(toast);
		return true;
	}
	return false;
}

std::string getCustomCollectionConfigPath(std::string collectionName)
{
	return getCollectionsFolder() + "/custom-" + collectionName + ".cfg";
}

std::string getCollectionsFolder()
{
	return Utils::FileSystem::getGenericPath(Utils::FileSystem::getHomePath() + "/.emulationstation/collections");
}

bool systemSort(SystemData* sys1, SystemData* sys2)
{
	std::string name1 = Utils::String::toUpper(sys1->getName());
	std::string name2 = Utils::String::toUpper(sys2->getName());
	return name1.compare(name2) < 0;
}
