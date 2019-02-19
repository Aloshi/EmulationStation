#pragma once

#include "pugixml/pugixml.hpp"
#include <string>
#include <map>
#include "GuiComponent.h"
#include "Util.h"
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

enum MetaDataType
{
	//generic types
	MD_STRING,
	MD_INT,
	MD_FLOAT,

	//specialized types
	MD_MULTILINE_STRING,
	MD_IMAGE_PATH,
	MD_RATING,
	MD_DATE,
	MD_TIME //used for lastplayed
};

struct MetaDataDecl
{
	std::string key;
	MetaDataType type;
	std::string defaultValue;
	bool isStatistic; //if true, ignore scraper values for this metadata
	std::string displayName; // displayed as this in editors
	std::string displayPrompt; // phrase displayed in editors when prompted to enter value (currently only for strings)
};

enum MetaDataListType
{
	GAME_METADATA,
	FOLDER_METADATA
};

const std::map< MetaDataListType, std::vector<MetaDataDecl> >& getMDDMap();

// this is just a dumb map meant to hold metadata records
class MetaDataMap
{
public:
	MetaDataMap(MetaDataListType type);

	inline MetaDataListType getType() const { return mType; }
	inline const std::vector<MetaDataDecl>& getMDD() const { return getMDDMap().at(getType()); }

	const std::string& get(const std::string& key) const
	{
		return mMap.at(key);
	}

	// data getters (see .cpp file for specializations)
	template<typename T>
	T get(const char* key) const
	{
		return boost::lexical_cast<T>(mMap.at(key));
	}

	template<typename T>
	T get(const std::string& key) const
	{
		return get<T>(key.c_str());
	}

	// data setters (see .cpp file for specializations)
	template<typename T>
	void set(const char* key, const T& value)
	{
		mMap[key] = boost::lexical_cast<std::string>(value);
	}

	template<typename T>
	void set(const std::string& key, const T& value)
	{
		set<T>(key.c_str(), value);
	}

private:
	MetaDataListType mType;
	std::map<std::string, std::string> mMap;
};

template<>
inline boost::posix_time::ptime MetaDataMap::get(const char* key) const
{
	return string_to_ptime(mMap.at(key), SQLITE_TIME_STRING_FORMAT);
}

template<>
inline void MetaDataMap::set(const char* key, const boost::posix_time::ptime& time)
{
	mMap[key] = boost::posix_time::to_iso_string(time);
}

