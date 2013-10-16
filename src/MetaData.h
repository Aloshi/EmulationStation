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
	MD_IMAGE_PATH_LIST,
	MD_RATING,
	MD_DATE,
	MD_TIME, //used for lastplayed
        MD_SELECTED // last selected item
};

struct MetaDataDecl
{
	std::string key;
	MetaDataType type;
	std::string defaultValue;
	bool isStatistic; //if true, ignore scraper values for this metadata
        bool isInternal; //if true, hide in metadata editor
};

boost::posix_time::ptime string_to_ptime(const std::string& str, const std::string& fmt = "%Y%m%dT%H%M%S%F%q");

class MetaDataList
{
public:
	static std::vector<MetaDataDecl> getDefaultGameMDD();

	static MetaDataList createFromXML(const std::vector<MetaDataDecl>& mdd, pugi::xml_node node);

	//MetaDataDecl required to set our defaults.
	MetaDataList(const std::vector<MetaDataDecl>& mdd);

        // accessor methods for list values (i.e. images)
        unsigned int getSize(const std::string &key);
        const std::string &getElemAt(const std::string &key, unsigned int pos);
        void clearList(const std::string &key);
        void push_back(const std::string &key, const std::string &value);
        void set(const std::string &key, unsigned int npos, const std::string &value);

	void set(const std::string& key, const std::string& value);
	void setTime(const std::string& key, const boost::posix_time::ptime& time); //times are internally stored as ISO strings (e.g. boost::posix_time::to_iso_string(ptime))

	const std::string& get(const std::string& key) const;
	int getInt(const std::string& key) const;
	float getFloat(const std::string& key) const;
	boost::posix_time::ptime getTime(const std::string& key) const;

	static GuiComponent* makeDisplay(Window* window, MetaDataType as);
	static GuiComponent* makeEditor(Window* window, MetaDataType as);

	void appendToXML(pugi::xml_node parent, const std::vector<MetaDataDecl>& ignoreDefaults = std::vector<MetaDataDecl>()) const;

private:
	MetaDataList();

	std::map<std::string, std::string> mMap;
};



//options for storing metadata...
//store internally everything as a string - this is all going to be read to/from XML anyway, after all
//store using individual get/set functions ala Settings - this is a fair amount of work but the most explicit, for better or worse

//let's think about some of the special types we would like to support...
//image paths, sound paths, ratings, play counts
//these get represented behind-the-scenes as strings, floats, and integers, and are eventually saved as strings
//the only specialty is how they're edited and viewed, really

//so we need...
//to be able to iterate through the available metadata
//create components designed to either DISPLAY or EDIT a given piece of metadata
//save and load metadata
