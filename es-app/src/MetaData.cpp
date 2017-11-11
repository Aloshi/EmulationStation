#include "MetaData.h"

#include "Log.h"
#include "Util.h"
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <pugixml/src/pugixml.hpp>

namespace fs = boost::filesystem;

MetaDataDecl gameDecls[] = {
	// key,         type,                   default,            statistic,  name in GuiMetaDataEd,  prompt in GuiMetaDataEd
	{"name",        MD_STRING,              "",                 false,      "name",                 "enter game name"},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      "description",          "enter description"},
	{"image",       MD_PATH,                "",                 false,      "image",                "enter path to image"},
	{"video",       MD_PATH     ,           "",                 false,      "video",                "enter path to video"},
	{"marquee",     MD_PATH,                "",                 false,      "marquee",              "enter path to marquee"},
	{"thumbnail",   MD_PATH,                "",                 false,      "thumbnail",            "enter path to thumbnail"},
	{"rating",      MD_RATING,              "0.000000",         false,      "rating",               "enter rating"},
	{"releasedate", MD_DATE,                "not-a-date-time",  false,      "release date",         "enter release date"},
	{"developer",   MD_STRING,              "unknown",          false,      "developer",            "enter game developer"},
	{"publisher",   MD_STRING,              "unknown",          false,      "publisher",            "enter game publisher"},
	{"genre",       MD_STRING,              "unknown",          false,      "genre",                "enter game genre"},
	{"players",     MD_INT,                 "1",                false,      "players",              "enter number of players"},
	{"favorite",    MD_BOOL,                "false",            false,      "favorite",             "enter favorite off/on"},
	{"hidden",      MD_BOOL,                "false",            false,      "hidden",               "enter hidden off/on" },
	{"kidgame",     MD_BOOL,                "false",            false,      "kidgame",              "enter kidgame off/on" },
	{"playcount",   MD_INT,                 "0",                true,       "play count",           "enter number of times played"},
	{"lastplayed",  MD_TIME,                "0",                true,       "last played",          "enter last played date"}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = {
	{"name",        MD_STRING,              "",                 false,      "name",                 "enter game name"},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      "description",          "enter description"},
	{"image",       MD_PATH,                "",                 false,      "image",                "enter path to image"},
	{"thumbnail",   MD_PATH,                "",                 false,      "thumbnail",            "enter path to thumbnail"},
	{"video",       MD_PATH,                "",                 false,      "video",                "enter path to video"},
	{"marquee",     MD_PATH,                "",                 false,      "marquee",              "enter path to marquee"},
	{"rating",      MD_RATING,              "0.000000",         false,      "rating",               "enter rating"},
	{"releasedate", MD_DATE,                "not-a-date-time",  false,      "release date",         "enter release date"},
	{"developer",   MD_STRING,              "unknown",          false,      "developer",            "enter game developer"},
	{"publisher",   MD_STRING,              "unknown",          false,      "publisher",            "enter game publisher"},
	{"genre",       MD_STRING,              "unknown",          false,      "genre",                "enter game genre"},
	{"players",     MD_INT,                 "1",                false,      "players",              "enter number of players"}
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
	: mType(type), mWasChanged(false)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node& node, const fs::path& relativeTo)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			// if it's a path, resolve relative paths
			std::string value = md.text().get();
			if (iter->type == MD_PATH)
			{
				value = resolvePath(value, relativeTo, true).generic_string();
			}
			mdl.set(iter->key, value);
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent, bool ignoreDefaults, const fs::path& relativeTo) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto mddIter = mdd.cbegin(); mddIter != mdd.cend(); mddIter++)
	{
		auto mapIter = mMap.find(mddIter->key);
		if(mapIter != mMap.cend())
		{
			// we have this value!
			// if it's just the default (and we ignore defaults), don't write it
			if(ignoreDefaults && mapIter->second == mddIter->defaultValue)
				continue;
			
			// try and make paths relative if we can
			std::string value = mapIter->second;
			if (mddIter->type == MD_PATH)
				value = makeRelativePath(value, relativeTo, true).generic_string();

			parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
		}
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
	mWasChanged = true;
}

void MetaDataList::setTime(const std::string& key, const boost::posix_time::ptime& time)
{
	set(key, boost::posix_time::to_iso_string(time));
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

bool MetaDataList::isDefault()
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for (unsigned int i = 1; i < mMap.size(); i++) {
		if (mMap.at(mdd[i].key) != mdd[i].defaultValue) return false;
	}

	return true;
}

bool MetaDataList::wasChanged() const
{
	return mWasChanged;
}

void MetaDataList::resetChangedFlag()
{
	mWasChanged = false;
}
