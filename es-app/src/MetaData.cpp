#include "MetaData.h"
#include "components/TextComponent.h"
#include "Log.h"
#include "Util.h"
#include <strings.h>
#include "Locale.h"

namespace fs = boost::filesystem;

std::vector<MetaDataDecl> gameMDD;
std::vector<MetaDataDecl> folderMDD;

void initMetadata() {
  // WARN : statistic metadata must be last in list !
  gameMDD.push_back(MetaDataDecl("emulator",	MD_LIST,		"default",		false,		_("Emulator"),			"enter emulator"));
  gameMDD.push_back(MetaDataDecl("core",	MD_LIST,		"default",		false,		_("Core"),			"enter core"));
  gameMDD.push_back(MetaDataDecl("name",	MD_STRING,		"", 			false,		_("Name"),			"enter game name"));
  gameMDD.push_back(MetaDataDecl("desc",	MD_MULTILINE_STRING,	"", 			false,		_("Description"),		"enter description"));
  gameMDD.push_back(MetaDataDecl("image",	MD_IMAGE_PATH,		"", 			false,		_("Image"),			"enter path to image"));
  gameMDD.push_back(MetaDataDecl("thumbnail",	MD_IMAGE_PATH,		"", 			false,		_("Thumbnail"),			"enter path to thumbnail"));
  gameMDD.push_back(MetaDataDecl("rating",	MD_RATING,		"0.000000", 		false,		_("Rating"),			"enter rating"));
  gameMDD.push_back(MetaDataDecl("releasedate", MD_DATE,		"not-a-date-time", 	false,		_("Release date"),		"enter release date"));
  gameMDD.push_back(MetaDataDecl("developer",	MD_STRING,		_("unknown"),		false,		_("Developer"),			"enter game developer"));
  gameMDD.push_back(MetaDataDecl("publisher",	MD_STRING,		_("unknown"),		false,		_("Publisher"),			"enter game publisher"));
  gameMDD.push_back(MetaDataDecl("genre",	MD_STRING,		_("unknown"),		false,		_("Genre"),			"enter game genre"));
  gameMDD.push_back(MetaDataDecl("players",	MD_INT,			"1",			false,		_("Players"),			"enter number of players"));
  gameMDD.push_back(MetaDataDecl("favorite",	MD_BOOL,		_("NO"),		false,		_("Favorite"),			"enter favorite"));
  gameMDD.push_back(MetaDataDecl("region",	MD_STRING,		"",			false,		_("Region"),			"enter region"));
  gameMDD.push_back(MetaDataDecl("romtype",	MD_STRING,		_("Original"),		false,		_("Romtype"),			"enter romtype"));
  gameMDD.push_back(MetaDataDecl("hidden",	MD_BOOL,		_("NO"),		false,		_("Hidden"),			"set hidden"));
  gameMDD.push_back(MetaDataDecl("playcount",	MD_INT,			"0",			true,		_("Play count"),		"enter number of times played"));
  gameMDD.push_back(MetaDataDecl("lastplayed",	MD_TIME,		"0", 			true,		_("Last played"),		"enter last played date"));

  folderMDD.push_back(MetaDataDecl("name",	MD_STRING,		"", 		false));
  folderMDD.push_back(MetaDataDecl("desc",	MD_MULTILINE_STRING,	"", 		false));
  folderMDD.push_back(MetaDataDecl("image",	MD_IMAGE_PATH,		"", 		false));
  folderMDD.push_back(MetaDataDecl("thumbnail",	MD_IMAGE_PATH,		"", 		false));
  folderMDD.push_back(MetaDataDecl("hidden",	MD_BOOL,		_("NO"),	false));
}

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


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node node, const fs::path& relativeTo)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			// if it's a path, resolve relative paths
			std::string value = md.text().get();
			if(iter->type == MD_IMAGE_PATH)
				value = resolvePath(value, relativeTo, true).generic_string();

			mdl.set(iter->key, value);
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node parent, bool ignoreDefaults, const fs::path& relativeTo) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto mddIter = mdd.begin(); mddIter != mdd.end(); mddIter++)
	{
		auto mapIter = mMap.find(mddIter->key);
		if(mapIter != mMap.end())
		{
			// we have this value!
			// if it's just the default (and we ignore defaults), don't write it
			if(ignoreDefaults && mapIter->second == mddIter->defaultValue)
				continue;
			
			// try and make paths relative if we can
			std::string value = mapIter->second;
			if(mddIter->type == MD_IMAGE_PATH)
				value = makeRelativePath(value, relativeTo, true).generic_string();

			parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
		}
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

void MetaDataList::merge(const MetaDataList& other) {
	const std::vector<MetaDataDecl> &mdd = getMDD();

	for (auto otherIter = other.mMap.begin(); otherIter != other.mMap.end(); otherIter++) {
		bool mustMerge = true;
		// Check if default value, if so continue
		for (auto mddIter = mdd.begin(); mddIter != mdd.end(); mddIter++) {
			if(mddIter->key == otherIter->first){
				if(otherIter->second == mddIter->defaultValue || mddIter->isStatistic){
					mustMerge = false;
				}
			}
		}
		if(mustMerge){
			this->set(otherIter->first, otherIter->second);
		}
	}
}
