#ifndef _GUITHEME_H_
#define _GUITHEME_H_

#include "../GuiComponent.h"
#include "../pugiXML/pugixml.hpp"
#include "GuiBox.h"
#include "../Sound.h"

//This class loads an XML-defined list of GuiComponents.
class GuiTheme : public GuiComponent
{
public:
	GuiTheme(std::string path = "");
	~GuiTheme();

	void readXML(std::string path);

	int getPrimaryColor();
	int getSecondaryColor();
	int getSelectorColor();
	int getSelectedTextColor();
	int getDescColor();
	int getFastSelectColor();
	bool getHeaderHidden();
	bool getDividersHidden();
	bool getListCentered();

	float getListOffsetX();
	float getListTextOffsetX();

	float getGameImageOffsetX();
	float getGameImageOffsetY();
	float getGameImageWidth();
	float getGameImageHeight();
	float getGameImageOriginX();
	float getGameImageOriginY();

	GuiBoxData getBoxData();

	Sound* getMenuScrollSound();
	Sound* getMenuSelectSound();
	Sound* getMenuBackSound();
	Sound* getMenuOpenSound();
private:
	void setDefaults();
	void deleteComponents();
	void createComponentChildren(pugi::xml_node node, GuiComponent* parent);
	GuiComponent* createElement(pugi::xml_node data, GuiComponent* parent);

	//utility functions
	std::string expandPath(std::string path);
	float resolveExp(std::string str, float defaultVal = 0.0);
	int resolveColor(std::string str, int defaultColor = 0x000000);
	void splitString(std::string str, char delim, std::string* before, std::string* after);
	float strToFloat(std::string str, float defaultVal = 0.0f);

	std::vector<GuiComponent*> mComponentVector;
	std::string mPath;
	int mListPrimaryColor, mListSecondaryColor, mListSelectorColor, mListSelectedColor, mDescColor, mFastSelectColor;
	bool mHideHeader, mHideDividers, mListCentered;

	float mListOffsetX, mListTextOffsetX, mGameImageOffsetX, mGameImageOffsetY, mGameImageWidth, mGameImageHeight, mGameImageOriginX, mGameImageOriginY;

	GuiBoxData mBoxData;

	Sound mMenuScrollSound, mMenuSelectSound, mMenuBackSound, mMenuOpenSound;
};

#endif
