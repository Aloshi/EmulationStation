#include "GuiTheme.h"
#include "../MathExp.h"
#include <iostream>
#include "GuiGameList.h"
#include "GuiImage.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include "../Renderer.h"

int GuiTheme::getPrimaryColor() { return mListPrimaryColor; }
int GuiTheme::getSecondaryColor() { return mListSecondaryColor; }
int GuiTheme::getSelectorColor() { return mListSelectorColor; }
int GuiTheme::getDescColor() { return mDescColor; }
int GuiTheme::getFastSelectColor() { return mFastSelectColor; }
bool GuiTheme::getHeaderHidden() { return mHideHeader; }
bool GuiTheme::getDividersHidden() { return mHideDividers; }
bool GuiTheme::getListCentered() { return mListCentered; }

float GuiTheme::getListOffsetX() { return mListOffsetX; }
float GuiTheme::getGameImageOffsetY() { return mGameImageOffsetY; }
float GuiTheme::getListTextOffsetX() { return mListTextOffsetX; }

int GuiTheme::getSelectedTextColor() { return mListSelectedColor; }

GuiBoxData GuiTheme::getBoxData() { return mBoxData; }

Sound* GuiTheme::getMenuScrollSound() { return &mMenuScrollSound; }
Sound* GuiTheme::getMenuSelectSound() { return &mMenuSelectSound; }
Sound* GuiTheme::getMenuBackSound() { return &mMenuBackSound; }
Sound* GuiTheme::getMenuOpenSound() { return &mMenuOpenSound; }

GuiTheme::GuiTheme(std::string path)
{
	setDefaults();

	if(!path.empty())
		readXML(path);
}

GuiTheme::~GuiTheme()
{
	deleteComponents();
}

void GuiTheme::setDefaults()
{
	mListPrimaryColor = 0x0000FF;
	mListSecondaryColor = 0x00FF00;
	mListSelectorColor = 0x000000;
	mListSelectedColor = -1; //-1 = use original list color when selected
	mDescColor = 0x0000FF;
	mFastSelectColor = 0xFF0000;
	mHideHeader = false;
	mHideDividers = false;
	mListCentered = true;

	mListOffsetX = 0.5;
	mListTextOffsetX = 0.005;
	mGameImageOffsetY = (float)Renderer::getFontHeight(Renderer::LARGE) / Renderer::getScreenHeight();

	mBoxData.backgroundPath = "";
	mBoxData.backgroundTiled = false;
	mBoxData.horizontalPath = "";
	mBoxData.horizontalTiled = false;
	mBoxData.verticalPath = "";
	mBoxData.verticalTiled = false;
	mBoxData.cornerPath = "";

	mMenuScrollSound.loadFile("");
	mMenuSelectSound.loadFile("");
	mMenuBackSound.loadFile("");
	mMenuOpenSound.loadFile("");
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

	setDefaults();
	deleteComponents();

	mPath = path;

	if(path.empty())
		return;

	//std::cout << "Loading theme \"" << path << "\"...\n";

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
	mListPrimaryColor = resolveColor(root.child("listPrimaryColor").text().get(), mListPrimaryColor);
	mListSecondaryColor = resolveColor(root.child("listSecondaryColor").text().get(), mListSecondaryColor);
	mListSelectorColor = resolveColor(root.child("listSelectorColor").text().get(), mListSelectorColor);
	mListSelectedColor = resolveColor(root.child("listSelectedColor").text().get(), mListSelectedColor);
	mDescColor = resolveColor(root.child("descColor").text().get(), mDescColor);
	mFastSelectColor = resolveColor(root.child("fastSelectColor").text().get(), mFastSelectColor);
	mHideHeader = root.child("hideHeader");
	mHideDividers = root.child("hideDividers");
	mListCentered = !root.child("listLeftAlign");

	//GuiBox theming data
	mBoxData.backgroundPath = expandPath(root.child("boxBackground").text().get());
	mBoxData.backgroundTiled = root.child("boxBackgroundTiled");
	mBoxData.horizontalPath = expandPath(root.child("boxHorizontal").text().get());
	mBoxData.horizontalTiled = root.child("boxHorizontalTiled");
	mBoxData.verticalPath = expandPath(root.child("boxVertical").text().get());
	mBoxData.verticalTiled = root.child("boxVerticalTiled");
	mBoxData.cornerPath = expandPath(root.child("boxCorner").text().get());

	mListOffsetX = strToFloat(root.child("listOffsetX").text().get(), mListOffsetX);
	mGameImageOffsetY = strToFloat(root.child("gameImageOffsetY").text().get(), mGameImageOffsetY);
	mListTextOffsetX = strToFloat(root.child("listTextOffsetX").text().get(), mListTextOffsetX);

	//sounds
	mMenuScrollSound.loadFile(expandPath(root.child("menuScrollSound").text().get()));
	mMenuSelectSound.loadFile(expandPath(root.child("menuSelectSound").text().get()));
	mMenuBackSound.loadFile(expandPath(root.child("menuBackSound").text().get()));
	mMenuOpenSound.loadFile(expandPath(root.child("menuOpenSound").text().get()));

	//recursively create children for all <components> with proper parenting
	createComponentChildren(root, this);
}

void GuiTheme::createComponentChildren(pugi::xml_node node, GuiComponent* parent)
{
	for(pugi::xml_node data = node.child("component"); data; data = data.next_sibling("component"))
	{
		GuiComponent* nextComp = createElement(data, parent);

		if(nextComp)
			createComponentChildren(data, nextComp);
	}
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

		bool tiled = data.child("tiled");

		//split position and dimension information
		std::string posX, posY;
		splitString(pos, ' ', &posX, &posY);

		std::string dimW, dimH;
		splitString(dim, ' ', &dimW, &dimH);

		std::string originX, originY;
		splitString(origin, ' ', &originX, &originY);

		//std::cout << "image, x: " << posX << " y: " << posY << " w: " << dimW << " h: " << dimH << " ox: " << originX << " oy: " << originY << " tiled: " << tiled << "\n";

		//resolve to pixels from percentages/variables
		int x = resolveExp(posX) * Renderer::getScreenWidth();
		int y = resolveExp(posY) * Renderer::getScreenHeight();
		int w = resolveExp(dimW) * Renderer::getScreenWidth();
		int h = resolveExp(dimH) * Renderer::getScreenHeight();

		float ox = strToFloat(originX);
		float oy = strToFloat(originY);

		GuiImage* comp = new GuiImage(x, y, "", w, h, true);
		comp->setOrigin(ox, oy);
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
	exp.setVariable("infoWidth", GuiGameList::sInfoWidth);

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

float GuiTheme::strToFloat(std::string str, float defaultVal)
{
	if(str.empty())
		return defaultVal;

	float ret;
	std::stringstream ss;
	ss << str;
	ss >> ret;
	return ret;
}
