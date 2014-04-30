#include "MetaData.h"
#include "components/TextComponent.h"
#include "Log.h"

MetaDataDecl gameDecls[] = { 
	// key,			type,					default,	statistic,	name in GuiMetaDataEd,	prompt in GuiMetaDataEd
	{"name",		MD_STRING,				"", 		false,		"name",					"enter game name"}, 
	{"desc",		MD_MULTILINE_STRING,	"", 		false,		"description",			"enter description"},
	{"image",		MD_IMAGE_PATH,			"", 		false,		"image",				"enter path to image"},
	{"thumbnail",	MD_IMAGE_PATH,			"", 		false,		"thumbnail",			"enter path to thumbnail"},
	{"rating",		MD_RATING,				"0", 		false,		"rating",				"enter rating"},
	{"releasedate", MD_DATE,				"0", 		false,		"release date",			"enter release date"},
	{"developer",	MD_STRING,				"unknown",	false,		"developer",			"enter game developer"},
	{"publisher",	MD_STRING,				"unknown",	false,		"publisher",			"enter game publisher"},
	{"genre",		MD_STRING,				"unknown",	false,		"genre",				"enter game genre"},
	{"players",		MD_INT,					"1",		false,		"players",				"enter number of players"},
	{"playcount",	MD_INT,					"0",		true,		"play count",			"enter number of times played"},
	{"lastplayed",	MD_TIME,				"0", 		true,		"last played",			"enter last played date"}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = { 
	{"name",		MD_STRING,				"", 	false}, 
	{"desc",		MD_MULTILINE_STRING,	"", 	false},
	{"image",		MD_IMAGE_PATH,			"", 	false},
	{"thumbnail",	MD_IMAGE_PATH,			"", 	false},
};
const std::vector<MetaDataDecl> folderMDD(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
	switch(type)
	{
	case GAME_METADATA:
		return gameMDD;
	case FOLDER_METADATA:
		return folderMDD;
	}

	LOG(LogError) << "Invalid MDD type";
	return gameMDD;
}



MetaDataList::MetaDataList(MetaDataListType type)
	: mType(type)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node node)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			mdl.set(iter->key, md.text().get());
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node parent, bool ignoreDefaults) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		bool write = true;

		if(ignoreDefaults)
		{
			for(auto mddIter = mdd.begin(); mddIter != mdd.end(); mddIter++)
			{
				if(mddIter->key == iter->first)
				{
					if(iter->second == mddIter->defaultValue)
						write = false;

					break;
				}
			}
		}

		if(write)
			parent.append_child(iter->first.c_str()).text().set(iter->second.c_str());
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
}

void MetaDataList::setTime(const std::string& key, const boost::posix_time::ptime& time)
{
	mMap[key] = boost::posix_time::to_iso_string(time);
}

const std::string& MetaDataList::get(const std::string& key) const
{
	return mMap.at(key);
}

int MetaDataList::getInt(const std::string& key) const
{
	return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
	return (float)atof(get(key).c_str());
}

boost::posix_time::ptime MetaDataList::getTime(const std::string& key) const
{
	return string_to_ptime(get(key), "%Y%m%dT%H%M%S%F%q");
}

//util function
boost::posix_time::ptime string_to_ptime(const std::string& str, const std::string& fmt)
{
	std::istringstream ss(str);
	ss.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(fmt))); //std::locale handles deleting the facet
	boost::posix_time::ptime time;
	ss >> time;

	return time;
}
