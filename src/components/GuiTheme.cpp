#include "GuiTheme.h"
#include "../MathExp.h"
#include <iostream>
#include "GuiImage.h"
#include <boost/filesystem.hpp>
#include <sstream>

int GuiTheme::getPrimaryColor() { return mListPrimaryColor; }
int GuiTheme::getSecondaryColor() { return mListSecondaryColor; }
int GuiTheme::getSelectorColor() { return mListSelectorColor; }
int GuiTheme::getDescColor() { return mDescColor; }
bool GuiTheme::getHeaderHidden() { return mHideHeader; }
bool GuiTheme::getDividersHidden() { return mHideDividers; }

GuiTheme::GuiTheme(std::string path)
{
	mListPrimaryColor = 0x0000FF;
	mListSecondaryColor = 0x00FF00;
	mListSelectorColor = 0x000000;
	mDescColor = 0x0000FF;
	mHideHeader = false;
	mHideDividers = false;

	if(!path.empty())
		readXML(path);
}

GuiTheme::~GuiTheme()
{
	deleteComponents();
}

void GuiTheme::deleteComponents()
{
	for(unsigned int i = 0; i < mComponentVector.size(); i++)
	{
		delete mComponentVector.at(i);
	}

	mComponentVector.clear();

	clearChildren();
}



void GuiTheme::readXML(std::string path)
{
	if(mPath == path)
		return;

	deleteComponents();

	mPath = path;

	if(path.empty())
		return;

	std::cout << "Loading theme \"" << path << "\"...\n";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.c_str());

	if(!result)
	{
		std::cerr << "Error parsing theme \"" << path << "\"!\n";
		std::cerr << "	" << result.description() << "\n";
		return;
	}

	pugi::xml_node root = doc.child("theme");

	//load non-component theme stuff
	mListPrimaryColor = resolveColor(root.child("listPrimaryColor").text().get(), 0x0000FF);
	mListSecondaryColor = resolveColor(root.child("listSecondaryColor").text().get(), 0x00FF00);
	mListSelectorColor = resolveColor(root.child("listSelectorColor").text().get(), 0x000000);
	mDescColor = resolveColor(root.child("descColor").text().get(), 0x0000FF);
	mHideHeader = root.child("hideHeader");
	mHideDividers = root.child("hideDividers");

	for(pugi::xml_node data = root.child("component"); data; data = data.next_sibling("component"))
	{
		createElement(data, this);
	}

	std::cout << "Finished parsing theme.\n";
}

GuiComponent* GuiTheme::createElement(pugi::xml_node data, GuiComponent* parent)
{
	std::string type = data.child("type").text().get();

	if(type == "image")
	{
		std::string path = expandPath(data.child("path").text().get());

		if(!boost::filesystem::exists(path))
		{
			std::cerr << "Error - theme image \"" << path << "\" does not exist.\n";
			return NULL;
		}

		std::string pos = data.child("pos").text().get();
		std::string dim = data.child("dim").text().get();
		std::string origin = data.child("origin").text().get();

		bool useAlpha = data.child("useAlpha");
		bool tiled = data.child("tiled");

		//split position and dimension information
		std::string posX, posY;
		splitString(pos, ' ', &posX, &posY);

		std::string dimW, dimH;
		splitString(dim, ' ', &dimW, &dimH);

		std::string originX, originY;
		splitString(origin, ' ', &originX, &originY);

		std::cout << "image, x: " << posX << " y: " << posY << " w: " << dimW << " h: " << dimH << " ox: " << originX << " oy: " << originY << " alpha: " << useAlpha << " tiled: " << tiled << "\n";

		//resolve to pixels from percentages/variables
		int x = resolveExp(posX) * Renderer::getScreenWidth();
		int y = resolveExp(posY) * Renderer::getScreenHeight();
		int w = resolveExp(dimW) * Renderer::getScreenWidth();
		int h = resolveExp(dimH) * Renderer::getScreenHeight();

		int ox = strToInt(originX);
		int oy = strToInt(originY);

		std::cout << "w: " << w << "px, h: " << h << "px\n";

		GuiImage* comp = new GuiImage(x, y, "", w, h, true);
		comp->setOrigin(ox, oy);
		comp->setAlpha(useAlpha);
		comp->setTiling(tiled);
		comp->setImage(path);

		parent->addChild(comp);
		mComponentVector.push_back(comp);
		return comp;
	}


	std::cerr << "Type \"" << type << "\" unknown!\n";
	return NULL;
}

std::string GuiTheme::expandPath(std::string path)
{
	if(path[0] == '~')
		path = getenv("HOME") + path.substr(1, path.length() - 1);
	else if(path[0] == '.')
		path = boost::filesystem::path(mPath).parent_path().string() + path.substr(1, path.length() - 1);

	return path;
}

float GuiTheme::resolveExp(std::string str)
{
	MathExp exp;
	exp.setExpression(str);

	//set variables
	exp.setVariable("headerHeight", Renderer::getFontHeight(Renderer::LARGE) / Renderer::getScreenHeight());

	return exp.eval();
}

int GuiTheme::resolveColor(std::string str, int defaultColor)
{
	if(str.empty())
		return defaultColor;

	int ret;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> ret;

	std::cout << "resolved " << str << " to " << ret << "\n";
	return ret;
}

void GuiTheme::splitString(std::string str, char delim, std::string* before, std::string* after)
{
	if(str.empty())
		return;

	size_t split = str.find(delim);
	*before = str.substr(0, split);
	*after = str.substr(split + 1, str.length() - split - 1);
}

int GuiTheme::strToInt(std::string str)
{
	if(str.empty())
		return 0;

	int ret;
	std::stringstream ss;
	ss << str;
	ss >> ret;
	return ret;
}
