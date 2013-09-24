#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>
#include <map>
#include "scrapers/Scraper.h"

//This is a singleton for storing settings.
class Settings
{
public:
	static Settings* getInstance();

	void loadFile();
	void saveFile();

	//You will get a warning if you try a get on a key that is not already present.
	bool getBool(const std::string& name);
	int getInt(const std::string& name);
	float getFloat(const std::string& name);

	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);

	Scraper* getScraper();
private:
	static Settings* sInstance;

	Settings();

	//Clear everything and load default values.
	void setDefaults();

	std::map<std::string, bool> mBoolMap;
	std::map<std::string, int> mIntMap;
	std::map<std::string, float> mFloatMap;
	Scraper* mScraper;
};

#endif
