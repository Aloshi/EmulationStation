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
float GuiTheme::getListTextOffsetX() { return mListTextOffsetX; }

int GuiTheme::getSelectedTextColor() { return mListSelectedColor; }

GuiBoxData GuiTheme::getBoxData() { return mBoxData; }

Sound* GuiTheme::getMenuScrollSound() { return &mMenuScrollSound; }
Sound* GuiTheme::getMenuSelectSound() { return &mMenuSelectSound; }
Sound* GuiTheme::getMenuBackSound() { return &mMenuBackSound; }
Sound* GuiTheme::getMenuOpenSound() { return &mMenuOpenSound; }

float GuiTheme::getGameImageOffsetX() { return mGameImageOffsetX; }
float GuiTheme::getGameImageOffsetY() { return mGameImageOffsetY; }
float GuiTheme::getGameImageWidth() { return mGameImageWidth; }
float GuiTheme::getGameImageHeight() { return mGameImageHeight; }
float GuiTheme::getGameImageOriginX() { return mGameImageOriginX; }
float GuiTheme::getGameImageOriginY() { return mGameImageOriginY; }

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

	mGameImageOriginX = 0.5;
	mGameImageOriginY = 0;
	mGameImageOffsetX = mListOffsetX / 2;
	mGameImageOffsetY = (float)Renderer::getFontHeight(Renderer::LARGE) / Renderer::getScreenHeight();
	mGameImageWidth = 0;
	mGameImageHeight = 0;

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

	//GuiBox theming data
	mBoxData.backgroundPath = expandPath(root.child("boxBackground").text().get());
	mBoxData.backgroundTiled = root.child("boxBackgroundTiled");
	mBoxData.horizontalPath = expandPath(root.child("boxHorizontal").text().get());
	mBoxData.horizontalTiled = root.child("boxHorizontalTiled");
	mBoxData.verticalPath = expandPath(root.child("boxVertical").text().get());
	mBoxData.verticalTiled = root.child("boxVerticalTiled");
	mBoxData.cornerPath = expandPath(root.child("boxCorner").text().get());

	//list stuff
	mListCentered = !root.child("listLeftAlign");
	mListOffsetX = strToFloat(root.child("listOffsetX").text().get(), mListOffsetX);
	mListTextOffsetX = strToFloat(root.child("listTextOffsetX").text().get(), mListTextOffsetX);


	//game image stuff
	std::string artPos = root.child("gameImagePos").text().get();
	std::string artDim = root.child("gameImageDim").text().get();
	std::string artOrigin = root.child("gameImageOrigin").text().get();

	std::string artPosX, artPosY, artWidth, artHeight, artOriginX, artOriginY;
	splitString(artPos, ' ', &artPosX, &artPosY);
	splitString(artDim, ' ', &artWidth, &artHeight);
	splitString(artOrigin, ' ', &artOriginX, &artOriginY);

	mGameImageOffsetX = resolveExp(artPosX, mGameImageOffsetX);
	mGameImageOffsetY = resolveExp(artPosY, mGameImageOffsetY);
	mGameImageWidth = resolveExp(artWidth, mGameImageWidth);
	mGameImageHeight = resolveExp(artHeight, mGameImageHeight);
	mGameImageOriginX = resolveExp(artOriginX, mGameImageOriginX);
	mGameImageOriginY = resolveExp(artOriginY, mGameImageOriginY);

	//sounds
	mMenuScrollSound.loadFile(expandPath(root.child("menuScrollSound").text().get()));
	mMenuSelectSound.loadFile(expandPath(root.child("menuSelectSound").text().get()));
	mMenuBackSound.loadFile(expandPath(root.child("menuBackSound").text().get()));
	mMenuOpenSound.loadFile(expandPath(root.child("menuOpenSound").text().get()));

	//actually read the components
	createComponentChildren(root, this);
}

//recursively creates components (with proper parenting)
void GuiTheme::createComponentChildren(pugi::xml_node node, GuiComponent* parent)
{
	for(pugi::xml_node data = node.child("component"); data; data = data.next_sibling("component"))
	{
		GuiComponent* nextComp = createElement(data, parent);

		if(nextComp)
			createComponentChildren(data, nextComp);
	}
}

//takes an XML element definition and creates an object from it
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


	std::cerr << "Theme component type \"" << type << "\" unknown!\n";
	return NULL;
}

//expands a file path (./ becomes the directory of this theme file, ~/ becomes $HOME/)
std::string GuiTheme::expandPath(std::string path)
{
	if(path[0] == '~')
		path = getenv("HOME") + path.substr(1, path.length() - 1);
	else if(path[0] == '.')
		path = boost::filesystem::path(mPath).parent_path().string() + path.substr(1, path.length() - 1);

	return path;
}

//takes a string containing a mathematical expression (possibly including variables) and resolves it to a float value
float GuiTheme::resolveExp(std::string str, float defaultVal)
{
	if(str.empty())
		return defaultVal;

	MathExp exp;
	exp.setExpression(str);

	//set variables
	exp.setVariable("headerHeight", Renderer::getFontHeight(Renderer::LARGE) / Renderer::getScreenHeight());
	exp.setVariable("infoWidth", mListOffsetX);

	return exp.eval();
}

//takes a string of hex and resolves it to an integer
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

//splits a string in two at the first instance of the delimiter
void GuiTheme::splitString(std::string str, char delim, std::string* before, std::string* after)
{
	if(str.empty())
		return;

	size_t split = str.find(delim);
	if(split != std::string::npos)
	{
		*before = str.substr(0, split);
		*after = str.substr(split + 1, str.length() - split - 1);
	}else{
		std::cerr << " Error: tried to splt string \"" << str << "\" with delimiter '" << delim << "', but delimiter was not found!\n";
	}
}

//converts a string to a float
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
