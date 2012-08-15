#ifndef _GUITHEME_H_
#define _GUITHEME_H_

#include "../GuiComponent.h"
#include "../pugiXML/pugixml.hpp"

class GuiTheme : public GuiComponent
{
public:
	GuiTheme(std::string path = "");
	~GuiTheme();

	void readXML(std::string path);

	int getPrimaryColor();
	int getSecondaryColor();
	int getSelectorColor();
	int getDescColor();
	bool getHeaderHidden();
	bool getDividersHidden();
	bool getListCentered();
private:
	void setDefaults();
	void deleteComponents();
	GuiComponent* createElement(pugi::xml_node data, GuiComponent* parent);

	//utility functions
	std::string expandPath(std::string path);
	float resolveExp(std::string str);
	int resolveColor(std::string str, int defaultColor = 0x000000);
	void splitString(std::string str, char delim, std::string* before, std::string* after);
	float strToFloat(std::string str);

	std::vector<GuiComponent*> mComponentVector;
	std::string mPath;
	int mListPrimaryColor, mListSecondaryColor, mListSelectorColor, mDescColor;
	bool mHideHeader, mHideDividers, mListCentered;
};

#endif
