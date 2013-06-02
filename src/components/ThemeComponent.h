#ifndef _THEMECOMPONENT_H_
#define _THEMECOMPONENT_H_

#include <memory>

#include "../GuiComponent.h"
#include "../pugiXML/pugixml.hpp"
#include "GuiBox.h"
#include "../AudioManager.h"
#include "../Font.h"

//This class loads an XML-defined list of GuiComponents.
class ThemeComponent : public GuiComponent
{
public:
	ThemeComponent(Window* window, bool detailed, std::string path = "");
	virtual ~ThemeComponent();

	void readXML(std::string path);

	GuiBoxData getBoxData();

	void init();
	void deinit();

	unsigned int getColor(std::string name);
	bool getBool(std::string name);
	float getFloat(std::string name);
	std::shared_ptr<Sound> & getSound(std::string name);
	std::string getString(std::string name);

	Font* getListFont();
	Font* getDescriptionFont();
	Font* getFastSelectFont();

private:
	void setDefaults();
	void deleteComponents();
	void createComponentChildren(pugi::xml_node node, GuiComponent* parent);
	GuiComponent* createElement(pugi::xml_node data, GuiComponent* parent);

	//utility functions
	std::string expandPath(std::string path);
	float resolveExp(std::string str, float defaultVal = 0.0);
	unsigned int resolveColor(std::string str, unsigned int defaultColor = 0x000000FF);
	void splitString(std::string str, char delim, std::string* before, std::string* after);
	float strToFloat(std::string str, float defaultVal = 0.0f);
	Font* resolveFont(pugi::xml_node node, std::string defaultPath, unsigned int defaultSize);

	std::string mPath;
	bool mDetailed;

	std::map<std::string, unsigned int> mColorMap;
	std::map<std::string, bool> mBoolMap;
	std::map<std::string, float> mFloatMap;
	std::map<std::string, std::shared_ptr<Sound>> mSoundMap;
	std::map<std::string, std::string> mStringMap;

	GuiBoxData mBoxData;

	Font* mListFont;
	Font* mDescFont;
	Font* mFastSelectFont;
};

#endif
