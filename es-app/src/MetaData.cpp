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
	{"lastplayed",	MD_TIME,				"0", 				true,		"last played",			"enter last played date"}
};

// because of how the GamelistDB is set up, this must be a subset of gameDecls
MetaDataDecl folderDecls[] = { 
	{"name",		MD_STRING,				"", 	false}, 
	{"desc",		MD_MULTILINE_STRING,	"", 	false},
	{"image",		MD_IMAGE_PATH,			"", 	false},
	{"thumbnail",	MD_IMAGE_PATH,			"", 	false},
};

std::map< MetaDataListType, std::vector<MetaDataDecl> > MDD_map = boost::assign::map_list_of
	(GAME_METADATA, 
		std::vector<MetaDataDecl>(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0])))
	(FOLDER_METADATA, 
		std::vector<MetaDataDecl>(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0])));
const std::map<MetaDataListType, std::vector<MetaDataDecl> >& getMDDMap()
{
	return MDD_map;
}

MetaDataMap::MetaDataMap(MetaDataListType type)
	: mType(type)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
		set(iter->key, iter->defaultValue);
}
