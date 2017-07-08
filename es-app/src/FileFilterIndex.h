#pragma once

#include <map>
#include "FileData.h"
#include "Log.h"
#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iostream>
#include "Util.h"

enum FilterIndexType
{
	NONE,
	GENRE_FILTER,
	PLAYER_FILTER,
	PUBDEV_FILTER,
	RATINGS_FILTER,
	FAVORITES_FILTER
};

struct FilterDataDecl
{
	FilterIndexType type; // type of filter
	std::map<std::string, int>* allIndexKeys; // all possible filters for this type
	bool* filteredByRef; // is it filtered by this type
	std::vector<std::string>* currentFilteredKeys; // current keys being filtered for
	std::string primaryKey; // primary key in metadata
	bool hasSecondaryKey; // has secondary key for comparison
	std::string secondaryKey; // what's the secondary key
	std::string menuLabel; // text to show in menu
};

class FileFilterIndex
{
public:
	FileFilterIndex();
	~FileFilterIndex();
	void addToIndex(FileData* game);
	void removeFromIndex(FileData* game);
	void setFilter(FilterIndexType type, std::vector<std::string>* values);
	void clearAllFilters();
	void debugPrintIndexes();
	bool showFile(FileData* game);
	bool isFiltered() { return (filterByGenre || filterByPlayers || filterByPubDev || filterByRatings || filterByFavorites); };
	bool isKeyBeingFilteredBy(std::string key, FilterIndexType type);
	std::vector<FilterDataDecl>& getFilterDataDecls();
private:
	std::vector<FilterDataDecl> filterDataDecl;
	std::string getIndexableKey(FileData* game, FilterIndexType type, bool getSecondary);

	void manageGenreEntryInIndex(FileData* game, bool remove = false);
	void managePlayerEntryInIndex(FileData* game, bool remove = false);
	void managePubDevEntryInIndex(FileData* game, bool remove = false);
	void manageRatingsEntryInIndex(FileData* game, bool remove = false);
	void manageFavoritesEntryInIndex(FileData* game, bool remove = false);

	void manageIndexEntry(std::map<std::string, int>* index, std::string key, bool remove);

	void clearIndex(std::map<std::string, int> indexMap);

	bool filterByGenre;
	bool filterByPlayers;
	bool filterByPubDev;
	bool filterByRatings;
	bool filterByFavorites;

	std::map<std::string, int> genreIndexAllKeys;
	std::map<std::string, int> playersIndexAllKeys;
	std::map<std::string, int> pubDevIndexAllKeys;
	std::map<std::string, int> ratingsIndexAllKeys;
	std::map<std::string, int> favoritesIndexAllKeys;

	std::vector<std::string> genreIndexFilteredKeys;
	std::vector<std::string> playersIndexFilteredKeys;
	std::vector<std::string> pubDevIndexFilteredKeys;
	std::vector<std::string> ratingsIndexFilteredKeys;
	std::vector<std::string> favoritesIndexFilteredKeys;

	FileData* mRootFolder;

};