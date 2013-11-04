#pragma once

#include "pugiXML/pugixml.hpp"
#include <string>
#include <map>
#include "GuiComponent.h"
#include <boost/date_time.hpp>

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
};

boost::posix_time::ptime string_to_ptime(const std::string& str, const std::string& fmt = "%Y%m%dT%H%M%S%F%q");

enum MetaDataListType
{
	GAME_METADATA,
	FOLDER_METADATA
};

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type);

class MetaDataList
{
public:
	static MetaDataList createFromXML(MetaDataListType type, pugi::xml_node node);
	void appendToXML(pugi::xml_node parent, bool ignoreDefaults = false) const;

	MetaDataList(MetaDataListType type);
	
	void set(const std::string& key, const std::string& value);
	void setTime(const std::string& key, const boost::posix_time::ptime& time); //times are internally stored as ISO strings (e.g. boost::posix_time::to_iso_string(ptime))

	const std::string& get(const std::string& key) const;
	int getInt(const std::string& key) const;
	float getFloat(const std::string& key) const;
	boost::posix_time::ptime getTime(const std::string& key) const;

	static GuiComponent* makeDisplay(Window* window, MetaDataType as);
	static GuiComponent* makeEditor(Window* window, MetaDataType as);

	inline MetaDataListType getType() const { return mType; }
	inline const std::vector<MetaDataDecl>& getMDD() const { return getMDDByType(getType()); }

private:
	MetaDataListType mType;
	std::map<std::string, std::string> mMap;
};



//options for storing metadata...
//store internally everything as a string - this is all going to be read to/from XML anyway, after all
//	- problem: this does not play nice with lists of values
//store using individual get/set functions ala Settings - this is a fair amount of work but the most explicit and type-safe, for better or worse

//let's think about some of the special types we would like to support...
//image paths, sound paths, ratings, play counts
//these get represented behind-the-scenes as strings, floats, and integers, and are eventually saved as strings
//the only specialty is how they're edited and viewed, really

//so we need...
//to be able to iterate through the available metadata
//create components designed to either DISPLAY or EDIT a given piece of metadata
//save and load metadata
