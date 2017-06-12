#include "FileFilterIndex.h"

#define UNKNOWN_LABEL "UNKNOWN"
#define INCLUDE_UNKNOWN false;

FileFilterIndex::FileFilterIndex()
	: filterByGenre(false), filterByPlayers(false), filterByPubDev(false), filterByRatings(false), filterByFavorites(false)
{
	FilterDataDecl filterDecls[] = {
		//type 				//allKeys 				//filteredBy 		//filteredKeys 				//primaryKey 	//hasSecondaryKey 	//secondaryKey 	//menuLabel
		{ FAVORITES_FILTER, &favoritesIndexAllKeys, &filterByFavorites,	&favoritesIndexFilteredKeys,"favorite",		false,				"",				"FAVORITES"	},
		{ GENRE_FILTER, 	&genreIndexAllKeys, 	&filterByGenre,		&genreIndexFilteredKeys, 	"genre",		true,				"genre",		"GENRE"	},
		{ PLAYER_FILTER, 	&playersIndexAllKeys, 	&filterByPlayers,	&playersIndexFilteredKeys, 	"players",		false,				"",				"PLAYERS"	},
		{ PUBDEV_FILTER, 	&pubDevIndexAllKeys, 	&filterByPubDev,	&pubDevIndexFilteredKeys, 	"developer",	true,				"publisher",	"PUBLISHER / DEVELOPER"	},
		{ RATINGS_FILTER, 	&ratingsIndexAllKeys, 	&filterByRatings,	&ratingsIndexFilteredKeys, 	"rating",		false,				"",				"RATING"	}
	};

	filterDataDecl = std::vector<FilterDataDecl>(filterDecls, filterDecls + sizeof(filterDecls) / sizeof(filterDecls[0]));
}

FileFilterIndex::~FileFilterIndex()
{
	clearIndex(genreIndexAllKeys);
	clearIndex(playersIndexAllKeys);
	clearIndex(pubDevIndexAllKeys);
	clearIndex(ratingsIndexAllKeys);
	clearIndex(favoritesIndexAllKeys);
}

std::vector<FilterDataDecl>& FileFilterIndex::getFilterDataDecls()
{
	return filterDataDecl;
}

std::string FileFilterIndex::getIndexableKey(FileData* game, FilterIndexType type, bool getSecondary)
{
	std::string key = "";
	switch(type)
	{
		case GENRE_FILTER:
		{
			key = strToUpper(game->metadata.get("genre"));
			boost::trim(key);
			if (getSecondary && !key.empty()) {
				std::istringstream f(key);
				std::string newKey;
				getline(f, newKey, '/');
				if (!newKey.empty() && newKey != key)
				{
					key = newKey;
				}
				else
				{
					key = std::string();
				}
			}
			break;
		}
		case PLAYER_FILTER:
		{
			if (getSecondary)
				break;

			key = game->metadata.get("players");
			break;
		}
		case PUBDEV_FILTER:
		{
			key = strToUpper(game->metadata.get("publisher"));
			boost::trim(key);

			if ((getSecondary && !key.empty()) || (!getSecondary && key.empty()))
				key = strToUpper(game->metadata.get("developer"));
			else
				key = strToUpper(game->metadata.get("publisher"));
			break;
		}
		case RATINGS_FILTER:
		{
			int ratingNumber = 0;
			if (!getSecondary)
			{
				std::string ratingString = game->metadata.get("rating");
				if (!ratingString.empty()) {
					try {
						ratingNumber = boost::math::iround(std::stod(ratingString)*5);
						if (ratingNumber < 0)
							ratingNumber = 0;

						key = std::to_string(ratingNumber) + " STARS";
					}
					catch (int e)
					{
						LOG(LogError) << "Error parsing Rating (invalid value, expected decimal): " << ratingString;
					}
				}
			}
			break;
		}
		case FAVORITES_FILTER:
		{
			if (game->getType() != GAME)
				return "FALSE";
			key = strToUpper(game->metadata.get("favorite"));
			break;
		}
	}
	boost::trim(key);
	if (key.empty() || (type == RATINGS_FILTER && key == "0 STARS")) {
		key = UNKNOWN_LABEL;
	}
	return key;
}

void FileFilterIndex::addToIndex(FileData* game)
{
	manageGenreEntryInIndex(game);
	managePlayerEntryInIndex(game);
	managePubDevEntryInIndex(game);
	manageRatingsEntryInIndex(game);
	manageFavoritesEntryInIndex(game);
}

void FileFilterIndex::removeFromIndex(FileData* game)
{
	manageGenreEntryInIndex(game, true);
	managePlayerEntryInIndex(game, true);
	managePubDevEntryInIndex(game, true);
	manageRatingsEntryInIndex(game, true);
	manageFavoritesEntryInIndex(game, true);
}

void FileFilterIndex::setFilter(FilterIndexType type, std::vector<std::string>* values)
{
	// test if it exists before setting
	if(type == NONE)
	{
		clearAllFilters();
	}
	else
	{
		for (std::vector<FilterDataDecl>::iterator it = filterDataDecl.begin(); it != filterDataDecl.end(); ++it ) {
			if ((*it).type == type)
			{
				FilterDataDecl filterData = (*it);
				*(filterData.filteredByRef) = values->size() > 0;
				filterData.currentFilteredKeys->clear();
				for (std::vector<std::string>::iterator vit = values->begin(); vit != values->end(); ++vit ) {
					// check if exists
					if (filterData.allIndexKeys->find(*vit) != filterData.allIndexKeys->end()) {
						filterData.currentFilteredKeys->push_back(std::string(*vit));
					}
				}
			}
		}
	}
	return;
}

void FileFilterIndex::clearAllFilters()
{
	for (std::vector<FilterDataDecl>::iterator it = filterDataDecl.begin(); it != filterDataDecl.end(); ++it )
	{
		FilterDataDecl filterData = (*it);
		*(filterData.filteredByRef) = false;
		filterData.currentFilteredKeys->clear();
	}
	return;
}

void FileFilterIndex::debugPrintIndexes()
{
	LOG(LogError) << "Printing Indexes...";
	for (auto x: playersIndexAllKeys) {
		LOG(LogError) << "Multiplayer Index: " << x.first << ": " << x.second;
	}
	for (auto x: genreIndexAllKeys) {
		LOG(LogError) << "Genre Index: " << x.first << ": " << x.second;
	}
	for (auto x: ratingsIndexAllKeys) {
		LOG(LogError) << "Ratings Index: " << x.first << ": " << x.second;
	}
	for (auto x: pubDevIndexAllKeys) {
		LOG(LogError) << "PubDev Index: " << x.first << ": " << x.second;
	}
	for (auto x: favoritesIndexAllKeys) {
		LOG(LogError) << "Favorites Index: " << x.first << ": " << x.second;
	}
}

bool FileFilterIndex::showFile(FileData* game)
{
	// this shouldn't happen, but just in case let's get it out of the way
	if (!isFiltered())
		return true;

	// if folder, needs further inspection - i.e. see if folder contains at least one element
	// that should be shown
	if (game->getType() == FOLDER) {
		std::vector<FileData*> children = game->getChildren();
		// iterate through all of the children, until there's a match

		for (std::vector<FileData*>::iterator it = children.begin(); it != children.end(); ++it ) {
			if (showFile(*it))
			{
				return true;
			}
		}
		return false;
	}

	bool keepGoing = false;

	for (std::vector<FilterDataDecl>::iterator it = filterDataDecl.begin(); it != filterDataDecl.end(); ++it ) {
		FilterDataDecl filterData = (*it);
		if(*(filterData.filteredByRef))
		{
			// try to find a match
			std::string key = getIndexableKey(game, filterData.type, false);
			keepGoing = isKeyBeingFilteredBy(key, filterData.type);

			// if we didn't find a match, try for secondary keys - i.e. publisher and dev, or first genre
			if (!keepGoing)
			{
				if (!filterData.hasSecondaryKey)
				{
					return false;
				}
				std::string secKey = getIndexableKey(game, filterData.type, true);
				if (secKey != UNKNOWN_LABEL)
				{
					keepGoing = isKeyBeingFilteredBy(secKey, filterData.type);
				}
			}
			// if still nothing, then it's not a match
			if (!keepGoing)
				return false;

		}

	}

	return keepGoing;
}

bool FileFilterIndex::isKeyBeingFilteredBy(std::string key, FilterIndexType type)
{
	const FilterIndexType filterTypes[5] = { FAVORITES_FILTER, PLAYER_FILTER, RATINGS_FILTER, GENRE_FILTER, PUBDEV_FILTER };
	std::vector<std::string> filterKeysList[5] = { favoritesIndexFilteredKeys, playersIndexFilteredKeys, ratingsIndexFilteredKeys, genreIndexFilteredKeys, pubDevIndexFilteredKeys };

	for (int i = 0; i < 5; i++)
	{
		if (filterTypes[i] == type)
		{
			for (std::vector<std::string>::iterator it = filterKeysList[i].begin(); it != filterKeysList[i].end(); ++it )
			{
				if (key == (*it))
				{
					return true;
				}
			}
			return false;
		}
	}

	return false;
}

void FileFilterIndex::manageGenreEntryInIndex(FileData* game, bool remove)
{

	std::string key = getIndexableKey(game, GENRE_FILTER, false);

	// flag for including unknowns
	bool includeUnknown = INCLUDE_UNKNOWN;

	// only add unknown in pubdev IF both dev and pub are empty
	if (!includeUnknown && (key == UNKNOWN_LABEL || key == "BIOS")) {
		// no valid genre info found
		return;
	}

	manageIndexEntry(&genreIndexAllKeys, key, remove);

	key = getIndexableKey(game, GENRE_FILTER, true);
	if (!includeUnknown && key == UNKNOWN_LABEL)
	{
		manageIndexEntry(&genreIndexAllKeys, key, remove);
	}
}

void FileFilterIndex::managePlayerEntryInIndex(FileData* game, bool remove)
{
	// flag for including unknowns
	bool includeUnknown = INCLUDE_UNKNOWN;
	std::string key = getIndexableKey(game, PLAYER_FILTER, false);

	// only add unknown in pubdev IF both dev and pub are empty
	if (!includeUnknown && key == UNKNOWN_LABEL) {
		// no valid player info found
		return;
	}

	manageIndexEntry(&playersIndexAllKeys, key, remove);
}

void FileFilterIndex::managePubDevEntryInIndex(FileData* game, bool remove)
{
	std::string pub = getIndexableKey(game, PUBDEV_FILTER, false);
	std::string dev = getIndexableKey(game, PUBDEV_FILTER, true);

	// flag for including unknowns
	bool includeUnknown = INCLUDE_UNKNOWN;
	bool unknownPub = false;
	bool unknownDev = false;

	if (pub == UNKNOWN_LABEL) {
		unknownPub = true;
	}
	if (dev == UNKNOWN_LABEL) {
		unknownDev = true;
	}

	if (!includeUnknown && unknownDev && unknownPub) {
		// no valid rating info found
		return;
	}

	if (unknownDev && unknownPub) {
		// if no info at all
		manageIndexEntry(&pubDevIndexAllKeys, pub, remove);
	}
	else
	{
		if (!unknownDev) {
			// if no info at all
			manageIndexEntry(&pubDevIndexAllKeys, dev, remove);
		}
		if (!unknownPub) {
			// if no info at all
			manageIndexEntry(&pubDevIndexAllKeys, pub, remove);
		}
	}
}

void FileFilterIndex::manageRatingsEntryInIndex(FileData* game, bool remove)
{
	std::string key = getIndexableKey(game, RATINGS_FILTER, false);

	// flag for including unknowns
	bool includeUnknown = INCLUDE_UNKNOWN;

	if (!includeUnknown && key == UNKNOWN_LABEL) {
		// no valid rating info found
		return;
	}

	manageIndexEntry(&ratingsIndexAllKeys, key, remove);
}

void FileFilterIndex::manageFavoritesEntryInIndex(FileData* game, bool remove)
{
	// flag for including unknowns
	bool includeUnknown = INCLUDE_UNKNOWN;
	std::string key = getIndexableKey(game, FAVORITES_FILTER, false);
	if (!includeUnknown && key == UNKNOWN_LABEL) {
		// no valid favorites info found
		return;
	}

	manageIndexEntry(&favoritesIndexAllKeys, key, remove);
}

void FileFilterIndex::manageIndexEntry(std::map<std::string, int>* index, std::string key, bool remove) {
	bool includeUnknown = INCLUDE_UNKNOWN;
	if (!includeUnknown && key == UNKNOWN_LABEL)
		return;
	if (remove) {
		// removing entry
		if (index->find(key) == index->end())
		{
			// this shouldn't happen
			LOG(LogInfo) << "Couldn't find entry in index! " << key;
		}
		else
		{
			(index->at(key))--;
			if(index->at(key) <= 0) {
				index->erase(key);
			}
		}
	}
	else
	{
		// adding entry
		if (index->find(key) == index->end())
		{
			(*index)[key] = 1;
		}
		else
		{
			(index->at(key))++;
		}
	}
}

void FileFilterIndex::clearIndex(std::map<std::string, int> indexMap)
{
	indexMap.clear();
}