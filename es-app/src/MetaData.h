#pragma once
#ifndef ES_APP_META_DATA_H
#define ES_APP_META_DATA_H

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/filesystem/path.hpp>
#include <pugixml/src/pugixml.hpp>

enum MetaDataType
{
	//generic types
	MD_STRING,
	MD_INT,
	MD_FLOAT,
	MD_BOOL,

	//specialized types
	MD_MULTILINE_STRING,
	MD_PATH,
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

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type);

class MetaDataList
{
public:
	static MetaDataList createFromXML(MetaDataListType type, pugi::xml_node node, const boost::filesystem::path& relativeTo);
	void appendToXML(pugi::xml_node parent, bool ignoreDefaults, const boost::filesystem::path& relativeTo) const;

	MetaDataList(MetaDataListType type);
	
	void set(const std::string& key, const std::string& value);
	void setTime(const std::string& key, const boost::posix_time::ptime& time); //times are internally stored as ISO strings (e.g. boost::posix_time::to_iso_string(ptime))

	const std::string& get(const std::string& key) const;
	int getInt(const std::string& key) const;
	float getFloat(const std::string& key) const;
	boost::posix_time::ptime getTime(const std::string& key) const;

	bool isDefault();

	bool wasChanged() const;
	void resetChangedFlag();

	inline MetaDataListType getType() const { return mType; }
	inline const std::vector<MetaDataDecl>& getMDD() const { return getMDDByType(getType()); }

private:
	MetaDataListType mType;
	std::map<std::string, std::string> mMap;
	bool mWasChanged;
};

#endif // ES_APP_META_DATA_H
