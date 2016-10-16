#include "Settings.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include "platform.h"
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>

Settings *Settings::sInstance = NULL;

// these values are NOT saved to es_settings.xml
// since they're set through command-line arguments, and not the in-program settings menu
std::vector<const char *> settings_dont_save = boost::assign::list_of
        ("Debug")
        ("DebugGrid")
        ("DebugText")
        ("ParseGamelistOnly")
        ("ShowExit")
        ("Windowed")
        ("VSync")
        ("HideConsole")
        ("IgnoreGamelist")
        ("UpdateCommand")
        ("UpdateServer")
        ("VersionFile")
        ("SharePartition")
        ("RecalboxSettingScript")
        ("RecalboxConfigScript")
        ("LastVersionFile")
        ("VersionMessage")
        ("MusicDirectory");

Settings::Settings() {
    setDefaults();
    loadFile();
}

Settings *Settings::getInstance() {
    if (sInstance == NULL)
        sInstance = new Settings();

    return sInstance;
}

void Settings::setDefaults() {
    mBoolMap.clear();
    mIntMap.clear();

    mBoolMap["BackgroundJoystickInput"] = false;
    mBoolMap["ParseGamelistOnly"] = false;
    mBoolMap["DrawFramerate"] = false;
    mBoolMap["ShowExit"] = true;
    mBoolMap["Windowed"] = false;

#ifdef _RPI_
	// don't enable VSync by default on the Pi, since it already
	// has trouble trying to render things at 60fps in certain menus
	mBoolMap["VSync"] = false;
    #else
    mBoolMap["VSync"] = true;
#endif

    mBoolMap["ShowHelpPrompts"] = true;
    mBoolMap["ScrapeRatings"] = true;
    mBoolMap["IgnoreGamelist"] = false;
    mBoolMap["HideConsole"] = true;
    mBoolMap["QuickSystemSelect"] = true;
    mBoolMap["FavoritesOnly"] = false;

    mBoolMap["Debug"] = false;
    mBoolMap["DebugGrid"] = false;
    mBoolMap["DebugText"] = false;


    mBoolMap["Overscan"] = false;

    mIntMap["ScreenSaverTime"] = 5 * 60 * 1000; // 5 minutes
    mIntMap["ScraperResizeWidth"] = 400;
    mIntMap["ScraperResizeHeight"] = 0;
    mIntMap["SystemVolume"] = 96;

    mStringMap["TransitionStyle"] = "fade";
    mStringMap["ThemeSet"] = "";
    mStringMap["ScreenSaverBehavior"] = "dim";
    mStringMap["Scraper"] = "TheGamesDB";
    mStringMap["Lang"] = "en_US";
    mStringMap["INPUT P1"] = "DEFAULT";
    mStringMap["INPUT P2"] = "DEFAULT";
    mStringMap["INPUT P3"] = "DEFAULT";
    mStringMap["INPUT P4"] = "DEFAULT";
    mStringMap["Overclock"] = "none";
    mStringMap["UpdateCommand"] = "/recalbox/scripts/recalbox-upgrade.sh";
    mStringMap["UpdateServer"] = "recalbox.com";
    mStringMap["VersionFile"] = "/recalbox/recalbox.version";
    mStringMap["SharePartition"] = "/recalbox/share/";
    mStringMap["RecalboxSettingScript"] = "/recalbox/scripts/recalbox-config.sh";
    mStringMap["LastVersionFile"] = "/recalbox/share/system/update.done";
    mStringMap["VersionMessage"] = "/recalbox/recalbox.msg";
    mStringMap["MusicDirectory"] = "/recalbox/share/music/";

}

template<typename K, typename V>
void saveMap(pugi::xml_node &node, std::map<K, V> &map, const char *type) {
    for (auto iter = map.begin(); iter != map.end(); iter++) {
        // key is on the "don't save" list, so don't save it
        if (std::find(settings_dont_save.begin(), settings_dont_save.end(), iter->first) != settings_dont_save.end())
            continue;

        pugi::xml_node parent_node = node.append_child(type);
        parent_node.append_attribute("name").set_value(iter->first.c_str());
        parent_node.append_attribute("value").set_value(iter->second);
    }
}

void Settings::saveFile() {
    const std::string path = getHomePath() + "/.emulationstation/es_settings.cfg";

    pugi::xml_document doc;

    pugi::xml_node config = doc.append_child("config");
    
    saveMap<std::string, bool>(config, mBoolMap, "bool");
    saveMap<std::string, int>(config, mIntMap, "int");
    saveMap<std::string, float>(config, mFloatMap, "float");

    //saveMap<std::string, std::string>(config, mStringMap, "string");
    for (auto iter = mStringMap.begin(); iter != mStringMap.end(); iter++) {
        pugi::xml_node node = config.append_child("string");
        node.append_attribute("name").set_value(iter->first.c_str());
        node.append_attribute("value").set_value(iter->second.c_str());
    }

    doc.save_file(path.c_str());
}

void Settings::loadFile() {
    const std::string path = getHomePath() + "/.emulationstation/es_settings.cfg";

    if (!boost::filesystem::exists(path))
        return;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result) {
        LOG(LogError) << "Could not parse Settings file!\n   " << result.description();
        return;
    }

    pugi::xml_node config = doc.child("config");
    if(config) { /* correct file format, having a config root node */
      for (pugi::xml_node node = config.child("bool"); node; node = node.next_sibling("bool"))
        setBool(node.attribute("name").as_string(), node.attribute("value").as_bool());
      for (pugi::xml_node node = config.child("int"); node; node = node.next_sibling("int"))
        setInt(node.attribute("name").as_string(), node.attribute("value").as_int());
      for (pugi::xml_node node = config.child("float"); node; node = node.next_sibling("float"))
        setFloat(node.attribute("name").as_string(), node.attribute("value").as_float());
      for (pugi::xml_node node = config.child("string"); node; node = node.next_sibling("string"))
        setString(node.attribute("name").as_string(), node.attribute("value").as_string());
    } else { /* the old format, without the root config node -- keep for a transparent upgrade */
      for (pugi::xml_node node = doc.child("bool"); node; node = node.next_sibling("bool"))
        setBool(node.attribute("name").as_string(), node.attribute("value").as_bool());
      for (pugi::xml_node node = doc.child("int"); node; node = node.next_sibling("int"))
        setInt(node.attribute("name").as_string(), node.attribute("value").as_int());
      for (pugi::xml_node node = doc.child("float"); node; node = node.next_sibling("float"))
        setFloat(node.attribute("name").as_string(), node.attribute("value").as_float());
      for (pugi::xml_node node = doc.child("string"); node; node = node.next_sibling("string"))
        setString(node.attribute("name").as_string(), node.attribute("value").as_string());
    }
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

SETTINGS_GETSET(const std::string&, mStringMap, getString, setString);
