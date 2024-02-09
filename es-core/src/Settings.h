#pragma once
#ifndef ES_CORE_SETTINGS_H
#define ES_CORE_SETTINGS_H

#include <map>
#include <string>

//This is a singleton for storing settings.
class Settings
{
public:
	static const int ONE_MINUTE_IN_MS = 1000 * 60;
	static Settings* getInstance();

	void loadFile();
	void saveFile();

	//You will get a warning if you try a get on a key that is not already present.
	bool getBool(const std::string& name);
	int getInt(const std::string& name);
	float getFloat(const std::string& name);
	const std::string& getString(const std::string& name);
	const std::map<std::string, int> getMap(const std::string& name);

	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);
	void setString(const std::string& name, const std::string& value);
	void setMap(const std::string& name, const std::map<std::string, int>& map);

private:
	static Settings* sInstance;

	Settings();

	void setDefaults();		//Clear everything and load default values.
	void processBackwardCompatibility();
	template<typename Map>
	void renameSetting(Map& map, std::string&& oldName, std::string&& newName);

	std::map<std::string, bool> mBoolMap;
	std::map<std::string, int> mIntMap;
	std::map<std::string, float> mFloatMap;
	std::map<std::string, std::string> mStringMap;
	std::map<std::string, std::map<std::string, int>> mMapIntMap;
};

#endif // ES_CORE_SETTINGS_H
