#include "ThemeComponent.h"
#include "../MathExp.h"
#include <iostream>
#include "GuiGameList.h"
#include "ImageComponent.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include "../Renderer.h"
#include "../Log.h"

unsigned int ThemeComponent::getColor(std::string name)
{
	return mColorMap[name];
}

bool ThemeComponent::getBool(std::string name)
{
	return mBoolMap[name];
}

float ThemeComponent::getFloat(std::string name)
{
	return mFloatMap[name];
}

std::shared_ptr<Sound> & ThemeComponent::getSound(std::string name)
{
	return mSoundMap[name];
}

std::string ThemeComponent::getString(std::string name)
{
	return mStringMap[name];
}

GuiBoxData ThemeComponent::getBoxData() { return mBoxData; }

std::shared_ptr<Font> ThemeComponent::getListFont()
{
	if(mListFont)
		return mListFont;
	else
		return Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM);
}

std::shared_ptr<Font> ThemeComponent::getDescriptionFont()
{
	if(mDescFont)
		return mDescFont;
	else
		return Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_SMALL);
}

std::shared_ptr<Font> ThemeComponent::getFastSelectFont()
{
	if(mFastSelectFont)
		return mFastSelectFont;
	else
		return Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_LARGE);
}

ThemeComponent::ThemeComponent(Window* window) : GuiComponent(window)
{
	mSoundMap["menuScroll"] = std::shared_ptr<Sound>(new Sound);
	mSoundMap["menuSelect"] = std::shared_ptr<Sound>(new Sound);
	mSoundMap["menuBack"] = std::shared_ptr<Sound>(new Sound);
	mSoundMap["menuOpen"] = std::shared_ptr<Sound>(new Sound);

	//register all sound with the audiomanager
	AudioManager::getInstance()->registerSound(mSoundMap["menuScroll"]);
	AudioManager::getInstance()->registerSound(mSoundMap["menuSelect"]);
	AudioManager::getInstance()->registerSound(mSoundMap["menuBack"]);
	AudioManager::getInstance()->registerSound(mSoundMap["menuOpen"]);

	setDefaults();
}

ThemeComponent::~ThemeComponent()
{
	deleteComponents();
}

void ThemeComponent::setDefaults()
{
	mColorMap["primary"] = 0x0000FFFF;
	mColorMap["secondary"] = 0x00FF00FF;
	mColorMap["selector"] = 0x000000FF;
	mColorMap["selected"] = 0x00000000;
	mColorMap["description"] = 0x0000FFFF;
	mColorMap["fastSelect"] = 0xFF0000FF;

	mBoolMap["hideHeader"] = false;
	mBoolMap["hideDividers"] = false;
	mBoolMap["listCentered"] = false;

	mFloatMap["listOffsetX"] = 0.5;
	mFloatMap["listTextOffsetX"] = 0.005f;
	mFloatMap["gameImageOriginX"] = 0.5;
	mFloatMap["gameImageOriginY"] = 0;
	mFloatMap["gameImageOffsetX"] = mFloatMap["listOffsetX"] / 2;
	mFloatMap["gameImageOffsetY"] = (float)FONT_SIZE_LARGE / (float)Renderer::getScreenHeight();
	mFloatMap["gameImageWidth"] = mFloatMap["listOffsetX"];
	mFloatMap["gameImageHeight"] = 0;

	mSoundMap["menuScroll"]->loadFile("");
	mSoundMap["menuSelect"]->loadFile("");
	mSoundMap["menuBack"]->loadFile("");
	mSoundMap["menuOpen"]->loadFile("");

	mStringMap["imageNotFoundPath"] = "";

	mBoxData.backgroundPath = "";
	mBoxData.backgroundTiled = false;
	mBoxData.horizontalPath = "";
	mBoxData.horizontalTiled = false;
	mBoxData.verticalPath = "";
	mBoxData.verticalTiled = false;
	mBoxData.cornerPath = "";

	mListFont.reset();
	mDescFont.reset();
	mFastSelectFont.reset();
}

void ThemeComponent::deleteComponents()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		delete getChild(i);
	}

	clearChildren();

	setDefaults();
}


void ThemeComponent::readXML(std::string path, bool detailed)
{
	if(mPath == path)
		return;

	setDefaults();
	deleteComponents();

	mPath = path;

	if(path.empty())
		return;

	LOG(LogInfo) << "Loading theme \"" << path << "\"...";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.c_str());

	if(!result)
	{
		LOG(LogError) << "Error parsing theme \"" << path << "\"!\n" << "	" << result.description();
		return;
	}

	pugi::xml_node root;

	if(!detailed)
	{
		//if we're using the basic view, check if there's a basic version of the theme
		root = doc.child("basicTheme");
	}

	if(!root)
	{
		root = doc.child("theme");
	}

	if(!root)
	{
		LOG(LogError) << "No theme tag found in theme \"" << path << "\"!";
		return;
	}

	//load non-component theme stuff
	mColorMap["primary"] = resolveColor(root.child("listPrimaryColor").text().get(), mColorMap["primary"]);
	mColorMap["secondary"] = resolveColor(root.child("listSecondaryColor").text().get(), mColorMap["secondary"]);
	mColorMap["selector"] = resolveColor(root.child("listSelectorColor").text().get(), mColorMap["selector"]);
	mColorMap["selected"] = resolveColor(root.child("listSelectedColor").text().get(), mColorMap["selected"]);
	mColorMap["description"] = resolveColor(root.child("descColor").text().get(), mColorMap["description"]);
	mColorMap["fastSelect"] = resolveColor(root.child("fastSelectColor").text().get(), mColorMap["fastSelect"]);

	mBoolMap["hideHeader"] = root.child("hideHeader") != 0;
	mBoolMap["hideDividers"] = root.child("hideDividers") != 0;

	//GuiBox theming data
	mBoxData.backgroundPath = expandPath(root.child("boxBackground").text().get());
	mBoxData.backgroundTiled = root.child("boxBackgroundTiled") != 0;
	mBoxData.horizontalPath = expandPath(root.child("boxHorizontal").text().get());
	mBoxData.horizontalTiled = root.child("boxHorizontalTiled") != 0;
	mBoxData.verticalPath = expandPath(root.child("boxVertical").text().get());
	mBoxData.verticalTiled = root.child("boxVerticalTiled") != 0;
	mBoxData.cornerPath = expandPath(root.child("boxCorner").text().get());

	//list stuff
	mBoolMap["listCentered"] = !root.child("listLeftAlign");
	mFloatMap["listOffsetX"] = strToFloat(root.child("listOffsetX").text().get(), mFloatMap["listOffsetX"]);
	mFloatMap["listTextOffsetX"] = strToFloat(root.child("listTextOffsetX").text().get(), mFloatMap["listTextOffsetX"]);

	//game image stuff
	std::string artPos = root.child("gameImagePos").text().get();
	std::string artDim = root.child("gameImageDim").text().get();
	std::string artOrigin = root.child("gameImageOrigin").text().get();

	std::string artPosX, artPosY, artWidth, artHeight, artOriginX, artOriginY;
	splitString(artPos, ' ', &artPosX, &artPosY);
	splitString(artDim, ' ', &artWidth, &artHeight);
	splitString(artOrigin, ' ', &artOriginX, &artOriginY);

	mFloatMap["gameImageOffsetX"] = resolveExp(artPosX, mFloatMap["gameImageOffsetX"]);
	mFloatMap["gameImageOffsetY"] = resolveExp(artPosY, mFloatMap["gameImageOffsetY"]);
	mFloatMap["gameImageWidth"] = resolveExp(artWidth, mFloatMap["gameImageWidth"]);
	mFloatMap["gameImageHeight"] = resolveExp(artHeight, mFloatMap["gameImageHeight"]);
	mFloatMap["gameImageOriginX"] = resolveExp(artOriginX, mFloatMap["gameImageOriginX"]);
	mFloatMap["gameImageOriginY"] = resolveExp(artOriginY, mFloatMap["gameImageOriginY"]);

	mStringMap["imageNotFoundPath"] = expandPath(root.child("gameImageNotFound").text().get());

	//sounds
	mSoundMap["menuScroll"]->loadFile(expandPath(root.child("menuScrollSound").text().get()));
	mSoundMap["menuSelect"]->loadFile(expandPath(root.child("menuSelectSound").text().get()));
	mSoundMap["menuBack"]->loadFile(expandPath(root.child("menuBackSound").text().get()));
	mSoundMap["menuOpen"]->loadFile(expandPath(root.child("menuOpenSound").text().get()));

	//fonts
	mListFont = resolveFont(root.child("listFont"), Font::getDefaultPath(), FONT_SIZE_MEDIUM);
	mDescFont = resolveFont(root.child("descriptionFont"), Font::getDefaultPath(), FONT_SIZE_SMALL);
	mFastSelectFont = resolveFont(root.child("fastSelectFont"), Font::getDefaultPath(), FONT_SIZE_LARGE);

	//actually read the components
	createComponentChildren(root, this);

	LOG(LogInfo) << "Theme loading complete.";
}

//recursively creates components
void ThemeComponent::createComponentChildren(pugi::xml_node node, GuiComponent* parent)
{
	for(pugi::xml_node data = node.child("component"); data; data = data.next_sibling("component"))
	{
		GuiComponent* nextComp = createElement(data, parent);

		if(nextComp)
			createComponentChildren(data, nextComp);
	}
}

//takes an XML element definition and creates an object from it
GuiComponent* ThemeComponent::createElement(pugi::xml_node data, GuiComponent* parent)
{
	std::string type = data.child("type").text().get();

	if(type == "image")
	{
		std::string path = expandPath(data.child("path").text().get());

		if(!boost::filesystem::exists(path))
		{
			LOG(LogError) << "Error - theme image \"" << path << "\" does not exist.";
			return NULL;
		}

		std::string pos = data.child("pos").text().get();
		std::string dim = data.child("dim").text().get();
		std::string origin = data.child("origin").text().get();

		bool tiled = data.child("tiled") != 0;

		//split position and dimension information
		std::string posX, posY;
		splitString(pos, ' ', &posX, &posY);

		std::string dimW, dimH;
		splitString(dim, ' ', &dimW, &dimH);

		std::string originX, originY;
		splitString(origin, ' ', &originX, &originY);

		//resolve to pixels from percentages/variables
		int x = (int)(resolveExp(posX) * Renderer::getScreenWidth());
		int y = (int)(resolveExp(posY) * Renderer::getScreenHeight());
		int w = (int)(resolveExp(dimW) * Renderer::getScreenWidth());
		int h = (int)(resolveExp(dimH) * Renderer::getScreenHeight());

		float ox = strToFloat(originX);
		float oy = strToFloat(originY);

		ImageComponent* comp = new ImageComponent(mWindow, x, y, "", w, h, true);
		comp->setOrigin(ox, oy);
		comp->setTiling(tiled);
		comp->setImage(path);

		addChild(comp);
		return comp;
	}


	LOG(LogError) << "Theme component type \"" << type << "\" unknown!";
	return NULL;
}

//expands a file path (./ becomes the directory of this theme file, ~/ becomes $HOME/)
std::string ThemeComponent::expandPath(std::string path)
{
	if(path.empty())
		return "";
	
	if(path[0] == '~')
		path = getHomePath() + path.substr(1, path.length() - 1);
	else if(path[0] == '.')
		path = boost::filesystem::path(mPath).parent_path().string() + path.substr(1, path.length() - 1);

	return path;
}

//takes a string containing a mathematical expression (possibly including variables) and resolves it to a float value
float ThemeComponent::resolveExp(std::string str, float defaultVal)
{
	if(str.empty())
		return defaultVal;

	MathExp exp;
	exp.setExpression(str);

	//set variables
	exp.setVariable("headerHeight", (float)(FONT_SIZE_LARGE / Renderer::getScreenHeight()));
	exp.setVariable("infoWidth", mFloatMap["listOffsetX"]);

	return exp.eval();
}

//takes a string of hex and resolves it to an integer
unsigned int ThemeComponent::resolveColor(std::string str, unsigned int defaultColor)
{
	if(str.empty())
		return defaultColor;

	if(str.length() != 6 && str.length() != 8)
	{
		LOG(LogError) << "Color \"" << str << "\" is not a valid hex color! Must be 6 or 8 characters.";
		return defaultColor;
	}

	//if there's no alpha specified, assume FF
	if(str.length() == 6)
		str += "FF";

	unsigned int ret;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> ret;

	return ret;
}

//splits a string in two at the first instance of the delimiter
void ThemeComponent::splitString(std::string str, char delim, std::string* before, std::string* after)
{
	if(str.empty())
		return;

	size_t split = str.find(delim);
	if(split != std::string::npos)
	{
		*before = str.substr(0, split);
		*after = str.substr(split + 1, str.length() - split - 1);
	}else{
		LOG(LogError) << "Tried to splt string \"" << str << "\" with delimiter '" << delim << "', but delimiter was not found!";
	}
}

//converts a string to a float
float ThemeComponent::strToFloat(std::string str, float defaultVal)
{
	if(str.empty())
		return defaultVal;

	float ret;
	std::stringstream ss;
	ss << str;
	ss >> ret;
	return ret;
}

std::shared_ptr<Font> ThemeComponent::resolveFont(pugi::xml_node node, std::string defaultPath, unsigned int defaultSize)
{
	if(!node)
		return NULL;

	std::string path = expandPath(node.child("path").text().get());
	unsigned int size = (unsigned int)(strToFloat(node.child("size").text().get()) * Renderer::getScreenHeight());

	if(!boost::filesystem::exists(path))
	{
		path = defaultPath;
	}

	if(size == 0)
	{
		size = defaultSize;
	}

	return Font::get(*mWindow->getResourceManager(), path, size);
}
