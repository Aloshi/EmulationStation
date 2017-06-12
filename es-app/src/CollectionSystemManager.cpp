#include "SystemData.h"
#include "Gamelist.h"
#include <boost/filesystem.hpp>
#include "Util.h"
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <SDL_joystick.h>
#include "Renderer.h"
#include "Log.h"
#include "InputManager.h"
#include <iostream>
#include "Settings.h"
#include "FileSorts.h"
#include "pugixml/src/pugixml.hpp"
#include "guis/GuiInfoPopup.h"

namespace fs = boost::filesystem;

CollectionSystemManager* CollectionSystemManager::sInstance = NULL;

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

CollectionSystemManager::CollectionSystemManager(Window* window) : mWindow(window)
{
	CollectionSystemDecl systemDecls[] = {
		//type                  name            long name       //default sort              // theme folder            // isCustom
		{ AUTO_ALL_GAMES,       "all",          "all games",    "filename, ascending",      "auto-allgames",           false },
		{ AUTO_LAST_PLAYED,     "recent",       "last played",  "last played, descending",  "auto-lastplayed",         false },
		{ AUTO_FAVORITES,       "favorites",    "favorites",    "filename, ascending",      "auto-favorites",          false },
		{ CUSTOM_COLLECTION,    "custom",       "custom",       "filename, ascending",      "custom-collections",      true }
	};

	// create a map
	std::vector<CollectionSystemDecl> tempSystemDecl = std::vector<CollectionSystemDecl>(systemDecls, systemDecls + sizeof(systemDecls) / sizeof(systemDecls[0]));

	for (std::vector<CollectionSystemDecl>::iterator it = tempSystemDecl.begin(); it != tempSystemDecl.end(); ++it )
	{
		mCollectionSystemDecls[(*it).name] = (*it);
		//mCollectionSystemDecls.insert(std::make_pair((*it).name,(*it)));
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

	// TO DO: Create custom editing help style

}

CollectionSystemManager::~CollectionSystemManager()
{
	assert(sInstance == this);
	sInstance = NULL;
}

void CollectionSystemManager::loadEnabledListFromSettings()
{
	// we parse the settings
	std::vector<std::string> selected = commaStringToVector(Settings::getInstance()->getString("CollectionSystemsAuto"));

	// iterate the map
	for(std::map<std::string, CollectionSystemData>::iterator it = mAllCollectionSystems.begin() ; it != mAllCollectionSystems.end() ; it++ )
	{
		it->second.isEnabled = (std::find(selected.begin(), selected.end(), it->first) != selected.end());
	}
}

void CollectionSystemManager::initAvailableSystemsList()
{

}

std::vector<std::string> CollectionSystemManager::getSystemsFromConfig()
{
	std::vector<std::string> systems;
	std::string path = SystemData::getConfigPath(false);

	if(!fs::exists(path))
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

std::vector<std::string> CollectionSystemManager::getSystemsFromTheme()
{
	std::vector<std::string> systems;
	auto themeSets = ThemeData::getThemeSets();
	if(themeSets.empty())
	{
		// no theme sets available
		return systems;
	}

	auto set = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
	if(set == themeSets.end())
	{
		// currently selected theme set is missing, so just pick the first available set
		set = themeSets.begin();
		Settings::getInstance()->setString("ThemeSet", set->first);
	}

	boost::filesystem::path themePath = set->second.path;

	if (fs::exists(themePath))
	{
		fs::directory_iterator end_itr; // default construction yields past-the-end
		for (fs::directory_iterator itr(themePath); itr != end_itr; ++itr)
		{
			if (fs::is_directory(itr->status()))
			{
				//... here you have a directory
				std::string folder = itr->path().string();
				folder = folder.substr(themePath.string().size()+1);

				if(fs::exists(set->second.getThemePath(folder)))
				{
					systems.push_back(folder);
				}
			}
		}
	}
	std::sort(systems.begin(), systems.end());
	return systems;
}

std::vector<std::string> CollectionSystemManager::getAutoThemeFolders()
{
	std::vector<std::string> systems;
	for(std::map<std::string, CollectionSystemDecl>::iterator it = mCollectionSystemDecls.begin() ; it != mCollectionSystemDecls.end() ; it++ )
	{
		CollectionSystemDecl sysDecl = it->second;
		if (!sysDecl.isCustom)
		{
			systems.push_back(sysDecl.themeFolder);
		}
	}
	return systems;
}

bool CollectionSystemManager::isThemeAutoCompatible()
{
	std::vector<std::string> cfgSys = getAutoThemeFolders();
	for(auto sysIt = cfgSys.begin(); sysIt != cfgSys.end(); sysIt++)
	{
		if(!themeFolderExists(*sysIt))
			return false;
	}
	return true;
}

bool CollectionSystemManager::themeFolderExists(std::string folder)
{
	std::vector<std::string> themeSys = getSystemsFromTheme();
	return std::find(themeSys.begin(), themeSys.end(), folder) != themeSys.end();
}

std::vector<std::string> CollectionSystemManager::getUnusedSystemsFromTheme()
{
	std::vector<std::string> cfgSys = getSystemsFromConfig();
	std::vector<std::string> themeSys = getSystemsFromTheme();
	for(auto sysIt = themeSys.begin(); sysIt != themeSys.end(); )
	{
		if (std::find(cfgSys.begin(), cfgSys.end(), *sysIt) != cfgSys.end())
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

FileData::SortType CollectionSystemManager::getSortType(std::string desc) {
	std::vector<FileData::SortType> SortTypes = FileSorts::SortTypes;
	// find it
	for(unsigned int i = 0; i < FileSorts::SortTypes.size(); i++)
	{
		const FileData::SortType& sort = FileSorts::SortTypes.at(i);
		if(sort.description == desc)
		{
			return sort;
		}
	}
	// if not found default to name, ascending
	return FileSorts::SortTypes.at(0);
}

void CollectionSystemManager::loadCollectionSystems()
{
	loadAutoCollectionSystems();
	// we will also load custom systems here in the future
	loadCustomCollectionSystems();
	// Now see which ones are enabled
	loadEnabledListFromSettings();
	// add to the main System Vector, and create Views as needed
	updateSystemsList();
}

void CollectionSystemManager::updateSystemsList()
{
	// remove all Collection Systems
	for(auto sysIt = SystemData::sSystemVector.begin(); sysIt != SystemData::sSystemVector.end(); )
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

	// add enabled ones
	for(std::map<std::string, CollectionSystemData>::iterator it = mAllCollectionSystems.begin() ; it != mAllCollectionSystems.end() ; it++ )
	{
		if(it->second.isEnabled)
		{
			SystemData::sSystemVector.push_back(it->second.system);
		}
	}

	// remove disabled gamelist views
	// create gamelist views if needed
	// iterate the map
}

void CollectionSystemManager::loadAutoCollectionSystems()
{
	for(std::map<std::string, CollectionSystemDecl>::iterator it = mCollectionSystemDecls.begin() ; it != mCollectionSystemDecls.end() ; it++ )
	{
		CollectionSystemDecl sysDecl = it->second;
		if (!sysDecl.isCustom && !findCollectionSystem(sysDecl.name))
		{
			SystemData* newSys = new SystemData(sysDecl.name, sysDecl.longName, mCollectionEnvData, sysDecl.themeFolder, true);

			FileData* rootFolder = newSys->getRootFolder();
			FileFilterIndex* index = newSys->getIndex();
			for(auto sysIt = SystemData::sSystemVector.begin(); sysIt != SystemData::sSystemVector.end(); sysIt++)
			{
				if ((*sysIt)->isGameSystem()) {
					std::vector<FileData*> files = (*sysIt)->getRootFolder()->getFilesRecursive(GAME);
					for(auto gameIt = files.begin(); gameIt != files.end(); gameIt++)
					{
						bool include = includeFileInAutoCollections((*gameIt));
						switch(sysDecl.type) {
							case AUTO_LAST_PLAYED:
								include = include && (*gameIt)->metadata.get("playcount") > "0";
								break;
							case AUTO_FAVORITES:
								// we may still want to add files we don't want in auto collections in "favorites"
								include = (*gameIt)->metadata.get("favorite") == "true";
								break;
						}

						if (include) {
							CollectionFileData* newGame = new CollectionFileData(*gameIt, newSys);
							rootFolder->addChild(newGame);
							index->addToIndex(newGame);
						}
					}
				}
			}
			rootFolder->sort(getSortType(sysDecl.defaultSort));
			mAutoCollectionSystems.push_back(newSys);

			CollectionSystemData newCollectionData;
			newCollectionData.system = newSys;
			newCollectionData.decl = sysDecl;
			newCollectionData.isEnabled = false;
			mAllCollectionSystems[sysDecl.name] = newCollectionData;
		}
	}
}

void CollectionSystemManager::loadCustomCollectionSystems()
{
	// Load custom systems into memory
	//			Check unassigned theme folders
	//			Check settings string for selected/enabled systems, if there are any that aren't in the theme
	//			Check saved preferences for each of those and load them if we can
	//		Load settings to see which systems are Enabled
}

SystemData* CollectionSystemManager::findCollectionSystem(std::string name)
{
	for(auto sysIt = SystemData::sSystemVector.begin(); sysIt != SystemData::sSystemVector.end(); sysIt++)
	{
		if ((*sysIt)->getName() == name) {
			// found it!
			return (*sysIt);
		}
	}
	return NULL;
}

// this updates all collection files related to the argument file
void CollectionSystemManager::updateCollectionSystems(FileData* file)
{
	// collection files use the full path as key, to avoid clashes
	std::string key = file->getFullPath();
	// find games in collection systems
	for(auto sysIt = SystemData::sSystemVector.begin(); sysIt != SystemData::sSystemVector.end(); sysIt++)
	{
		if ((*sysIt)->isCollection()) {
			const std::unordered_map<std::string, FileData*>& children = (*sysIt)->getRootFolder()->getChildrenByFilename();
			bool found = children.find(key) != children.end();
			FileData* rootFolder = (*sysIt)->getRootFolder();
			FileFilterIndex* fileIndex = (*sysIt)->getIndex();
			std::string name = (*sysIt)->getName();
			if (found) {
				// if we found it, we need to update it
				FileData* collectionEntry = children.at(key);
				// remove from index, so we can re-index metadata after refreshing
				fileIndex->removeFromIndex(collectionEntry);
				collectionEntry->refreshMetadata();
				if (name == "favorites" && file->metadata.get("favorite") == "false") {
					// need to check if still marked as favorite, if not remove
					ViewController::get()->getGameListView((*sysIt)).get()->remove(collectionEntry, false);
					ViewController::get()->onFileChanged((*sysIt)->getRootFolder(), FILE_REMOVED);
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
				if (name == "recent" && file->metadata.get("playcount") > "0" ||
					name == "favorites" && file->metadata.get("favorite") == "true") {
					CollectionFileData* newGame = new CollectionFileData(file, (*sysIt));
					rootFolder->addChild(newGame);
					fileIndex->addToIndex(newGame);
					ViewController::get()->onFileChanged(file, FILE_METADATA_CHANGED);
					ViewController::get()->getGameListView((*sysIt))->onFileChanged(newGame, FILE_METADATA_CHANGED);
				}
			}
			rootFolder->sort(getSortType(mCollectionSystemDecls[name].defaultSort));
			ViewController::get()->onFileChanged(rootFolder, FILE_SORTED);
		}
	}
}

// this deletes collection files from collection systems
void CollectionSystemManager::deleteCollectionFiles(FileData* file)
{
	// collection files use the full path as key, to avoid clashes
	std::string key = file->getFullPath();
	// find games in collection systems
	for(auto sysIt = SystemData::sSystemVector.begin(); sysIt != SystemData::sSystemVector.end(); sysIt++)
	{
		if ((*sysIt)->isCollection()) {
			const std::unordered_map<std::string, FileData*>& children = (*sysIt)->getRootFolder()->getChildrenByFilename();

			bool found = children.find(key) != children.end();
			if (found) {
				FileData* collectionEntry = children.at(key);
				ViewController::get()->getGameListView((*sysIt)).get()->remove(collectionEntry, false);
			}
		}
	}
}

bool CollectionSystemManager::toggleGameInCollection(FileData* file, std::string collection)
{
	if (file->getType() == GAME)
	{
		GuiInfoPopup* s;

		MetaDataList* md = &file->getSourceFileData()->metadata;
		std::string value = md->get("favorite");
		if (value == "false")
		{
			md->set("favorite", "true");
			s = new GuiInfoPopup(mWindow, "Added '" + removeParenthesis(file->getName()) + "' to 'Favorites'", 4000);
		}else
		{
			md->set("favorite", "false");
			s = new GuiInfoPopup(mWindow, "Removed '" + removeParenthesis(file->getName()) + "' from 'Favorites'", 4000);
		}
		mWindow->setInfoPopup(s);
		updateCollectionSystems(file->getSourceFileData());
		return true;
	}
	return false;
}

bool CollectionSystemManager::includeFileInAutoCollections(FileData* file)
{
	// we exclude non-game files from collections (i.e. "kodi", at least)
	// if/when there are more in the future, maybe this can be a more complex method, with a proper list
	// but for now a simple string comparison is more performant
	return file->getName() != "kodi";
}