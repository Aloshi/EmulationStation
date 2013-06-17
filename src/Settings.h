#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>
#include <map>

//This is a singleton for storing settings.
class Settings
{
public:
	static Settings* getInstance();

	void loadFile(const std::string& path);
	void saveFile(const std::string& path);

	//You will get a warning if you try a get on a key that is not already present.
	bool getBool(const std::string& name);
	int getInt(const std::string& name);

	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);

private:
	static Settings* sInstance;

	Settings();

	//Clear everything and load default values.
	void setDefaults();

	std::map<std::string, bool> mBoolMap;
	std::map<std::string, int> mIntMap;
};

#endif
