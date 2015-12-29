#include "MetaData.h"
#include <boost/assign.hpp>

MetaDataDecl gameDecls[] = { 
	// key,			type,					default,			statistic,	name in GuiMetaDataEd,	prompt in GuiMetaDataEd
	{"name",		MD_STRING,				"", 				false,		"name",					"enter game name"}, 
	{"desc",		MD_MULTILINE_STRING,	"", 				false,		"description",			"enter description"},
	{"image",		MD_IMAGE_PATH,			"", 				false,		"image",				"enter path to image"},
	{"thumbnail",	MD_IMAGE_PATH,			"", 				false,		"thumbnail",			"enter path to thumbnail"},
	{"rating",		MD_RATING,				"0.000000", 		false,		"rating",				"enter rating"},
	{"releasedate", MD_DATE,				"not-a-date-time", 	false,		"release date",			"enter release date"},
	{"developer",	MD_STRING,				"unknown",			false,		"developer",			"enter game developer"},
	{"publisher",	MD_STRING,				"unknown",			false,		"publisher",			"enter game publisher"},
	{"genre",		MD_STRING,				"unknown",			false,		"genre",				"enter game genre"},
	{"players",		MD_INT,					"1",				false,		"players",				"enter number of players"},
	{"playcount",	MD_INT,					"0",				true,		"play count",			"enter number of times played"},
	{"lastplayed",	MD_TIME,				"not-a-date-time", 				true,		"last played",			"enter last played date"}
};

// because of how the GamelistDB is set up, this must be a subset of gameDecls
MetaDataDecl folderDecls[] = { 
	{"name",		MD_STRING,				"", 				false,		"name",					"enter game name"}, 
	{"desc",		MD_MULTILINE_STRING,	"", 				false,		"description",			"enter description"},
	{"image",		MD_IMAGE_PATH,			"", 				false,		"image",				"enter path to image"},
	{"thumbnail",	MD_IMAGE_PATH,			"", 				false,		"thumbnail",			"enter path to thumbnail"},
};
// because of that subset constraint, note the abuse of the genre field
// It is marked as MD_MULTILINE_STRING instead of MD_STRING. This appears to be ok.
MetaDataDecl filterDecls[] = { 
	{"name",		MD_STRING,				"Filter", 				false,		"name",					"enter filter name"}, 
	{"desc",		MD_MULTILINE_STRING,	"", 				false,		"description",			"enter description"},
	{"image",		MD_IMAGE_PATH,			"", 				false,		"image",				"enter path to image"},
	{"thumbnail",	MD_IMAGE_PATH,			"", 				false,		"thumbnail",			"enter path to thumbnail"},
        {"genre",	MD_MULTILINE_STRING,			"rating > .6 AND playcount > 0", 				false,		"query",			"enter query"},
};

std::map< MetaDataListType, std::vector<MetaDataDecl> > MDD_map = boost::assign::map_list_of
	(GAME_METADATA, 
		std::vector<MetaDataDecl>(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0])))
	(FOLDER_METADATA, 
		std::vector<MetaDataDecl>(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0])))
	(FILTER_METADATA, 
		std::vector<MetaDataDecl>(filterDecls, filterDecls + sizeof(filterDecls) / sizeof(filterDecls[0])));

const std::map<MetaDataListType, std::vector<MetaDataDecl> >& getMDDMap()
{
	return MDD_map;
}

MetaDataMap::MetaDataMap(MetaDataListType type)
	: mType(type)
{
	//To enforce that subset constraint above, we intialize the map to have
	//all the defaults of the gameDecls, with our specific defaults overriding.
	const std::vector<MetaDataDecl>& mddGame = getMDDMap().at(GAME_METADATA);
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mddGame.begin(); iter != mddGame.end(); iter++)
		set(iter->key, iter->defaultValue); 
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
		set(iter->key, iter->defaultValue);
}
