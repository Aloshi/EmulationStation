#include "Settings.h"
#include "Log.h"
#include "pugiXML/pugixml.hpp"
#include "platform.h"
#include <boost/filesystem.hpp>
#include "scrapers/GamesDBScraper.h"

Settings* Settings::sInstance = NULL;

Settings::Settings()
{
	setDefaults();
	loadFile();
}

Settings* Settings::getInstance()
{
	if(sInstance == NULL)
		sInstance = new Settings();

	return sInstance;
}

void Settings::setDefaults()
{
	mBoolMap.clear();
	mIntMap.clear();

	mBoolMap["PARSEGAMELISTONLY"] = false;
	mBoolMap["IGNOREGAMELIST"] = false;
	mBoolMap["DRAWFRAMERATE"] = false;
	mBoolMap["DONTSHOWEXIT"] = false;
	mBoolMap["DEBUG"] = false;
	mBoolMap["WINDOWED"] = false;
	mBoolMap["DISABLESOUNDS"] = false;
	mBoolMap["DisableGamelistWrites"] = false;

	mIntMap["DIMTIME"] = 30*1000;
	mIntMap["ScraperResizeWidth"] = 450;
	mIntMap["ScraperResizeHeight"] = 0;

	mIntMap["GameListSortIndex"] = 0;


	mScraper = std::shared_ptr<Scraper>(new GamesDBScraper());
}

template <typename K, typename V>
void saveMap(pugi::xml_document& doc, std::map<K, V>& map, const char* type)
{
	for(auto iter = map.begin(); iter != map.end(); iter++)
	{
		pugi::xml_node node = doc.append_child(type);
		node.append_attribute("name").set_value(iter->first.c_str());
		node.append_attribute("value").set_value(iter->second);
	}
}

void Settings::saveFile()
{
	const std::string path = getHomePath() + "/.emulationstation/es_settings.cfg";

	pugi::xml_document doc;

	saveMap<std::string, bool>(doc, mBoolMap, "bool");
	saveMap<std::string, int>(doc, mIntMap, "int");
	saveMap<std::string, float>(doc, mFloatMap, "float");

	pugi::xml_node scraperNode = doc.append_child("scraper");
	scraperNode.append_attribute("value").set_value(mScraper->getName());

	doc.save_file(path.c_str());
}

void Settings::loadFile()
{
	const std::string path = getHomePath() + "/.emulationstation/es_settings.cfg";

	if(!boost::filesystem::exists(path))
		return;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.c_str());
	if(!result)
	{
		LOG(LogError) << "Could not parse Settings file!\n   " << result.description();
		return;
	}

	for(pugi::xml_node node = doc.child("bool"); node; node = node.next_sibling())
		setBool(node.attribute("name").as_string(), node.attribute("value").as_bool());
	for(pugi::xml_node node = doc.child("int"); node; node = node.next_sibling())
		setInt(node.attribute("name").as_string(), node.attribute("value").as_int());
	for(pugi::xml_node node = doc.child("float"); node; node = node.next_sibling())
		setFloat(node.attribute("name").as_string(), node.attribute("value").as_float());

	if(doc.child("scraper"))
	{
		std::shared_ptr<Scraper> scr = createScraperByName(doc.child("scraper").attribute("value").as_string());
		if(scr)
			mScraper = scr;
	}
}

std::shared_ptr<Scraper> Settings::getScraper()
{
	return mScraper;
}

void Settings::setScraper(std::shared_ptr<Scraper> scraper)
{
	mScraper = scraper;
}

//Print a warning message if the setting we're trying to get doesn't already exist in the map, then return the value in the map.
#define SETTINGS_GETSET(type, mapName, getMethodName, setMethodName) type Settings::getMethodName(const std::string& name) \
{ \
	if(mapName.find(name) == mapName.end()) \
	{ \
		LOG(LogError) << "Tried to use unset setting " << name << "!"; \
	} \
	return mapName[name]; \
} \
void Settings::setMethodName(const std::string& name, type value) \
{ \
	mapName[name] = value; \
}

SETTINGS_GETSET(bool, mBoolMap, getBool, setBool);
SETTINGS_GETSET(int, mIntMap, getInt, setInt);
SETTINGS_GETSET(float, mFloatMap, getFloat, setFloat);
