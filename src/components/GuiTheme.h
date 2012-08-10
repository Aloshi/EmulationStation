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

private:
	void deleteComponents();

	GuiComponent* createElement(pugi::xml_node data, GuiComponent* parent);
	int resolveExp(std::string str);

	std::vector<GuiComponent*> mComponentVector;
};

#endif
