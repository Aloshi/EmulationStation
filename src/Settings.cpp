#include "Settings.h"
#include "Log.h"

Settings* Settings::sInstance = NULL;

Settings::Settings()
{
	setDefaults();
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

	mIntMap["DIMTIME"] = 30*1000;
}

//Print a warning message if the setting we're trying to get doesn't already exist in the map, then return the value in the map.
#define SETTINGS_GET(type, mapName, methodName) type Settings::##methodName##(const std::string& name) \
{ \
	if(mapName.find(name) == mapName.end()) \
	{ \
		LOG(LogError) << "Tried to use unset setting " << name << "!"; \
	} \
	return mapName[name]; \
}

SETTINGS_GET(bool, mBoolMap, getBool);
SETTINGS_GET(int, mIntMap, getInt);

void Settings::setBool(const std::string& name, bool value) { mBoolMap[name] = value; }
void Settings::setInt(const std::string& name, int value) { mIntMap[name] = value; }
