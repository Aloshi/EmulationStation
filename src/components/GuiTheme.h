#ifndef _GUITHEME_H_
#define _GUITHEME_H_

#include <memory>

#include "../Gui.h"
#include "../pugiXML/pugixml.hpp"
#include "GuiBox.h"
#include "../AudioManager.h"
#include "../Font.h"

//This class loads an XML-defined list of Guis.
class GuiTheme : public Gui
{
public:
	GuiTheme(Window* window, bool detailed, std::string path = "");
	~GuiTheme();

	void readXML(std::string path);

	GuiBoxData getBoxData();

	void render();

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
	void createComponentChildren(pugi::xml_node node, Gui* parent);
	Gui* createElement(pugi::xml_node data, Gui* parent);

	//utility functions
	std::string expandPath(std::string path);
	float resolveExp(std::string str, float defaultVal = 0.0);
	unsigned int resolveColor(std::string str, unsigned int defaultColor = 0x000000FF);
	void splitString(std::string str, char delim, std::string* before, std::string* after);
	float strToFloat(std::string str, float defaultVal = 0.0f);
	Font* resolveFont(pugi::xml_node node, std::string defaultPath, unsigned int defaultSize);

	std::vector<Gui*> mComponentVector;
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
