#include "ThemeData.h"
#include "Renderer.h"
#include "resources/Font.h"
#include "Sound.h"
#include "resources/TextureResource.h"
#include "Log.h"
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>
#include "pugiXML/pugixml.hpp"

// Defaults
std::map<std::string, FontDef > ThemeData::sDefaultFonts = boost::assign::map_list_of
	("listFont", FontDef(0.045f, ""))
	("descriptionFont", FontDef(0.035f, ""));

std::map<std::string, unsigned int> ThemeData::sDefaultColors = boost::assign::map_list_of
	("listPrimaryColor", 0x0000FFFF)
	("listSecondaryColor", 0x00FF00FF)
	("listSelectorColor", 0x000000FF)
	("listSelectedColor", 0x00000000)
	("descriptionColor", 0x48474DFF);

std::map<std::string, ImageDef> ThemeData::sDefaultImages = boost::assign::map_list_of
	("backgroundImage", ImageDef("", true))
	("headerImage", ImageDef("", false))
	("infoBackgroundImage", ImageDef("", false));

std::map<std::string, SoundDef> ThemeData::sDefaultSounds = boost::assign::map_list_of
	("scrollSound", SoundDef(""))
	("gameSelectSound", SoundDef(""))
	("backSound", SoundDef(""))
	("menuOpenSound", SoundDef(""));



const std::shared_ptr<ThemeData>& ThemeData::getDefault()
{
	static const std::shared_ptr<ThemeData> def = std::shared_ptr<ThemeData>(new ThemeData());
	return def;
}

ThemeData::ThemeData()
{
	setDefaults();

	std::string defaultDir = getHomePath() + "/.emulationstation/es_theme_default.xml";
	if(boost::filesystem::exists(defaultDir))
		loadFile(defaultDir);
}

void ThemeData::setDefaults()
{
	mFontMap.clear();
	mImageMap.clear();
	mColorMap.clear();
	mSoundMap.clear();

	mFontMap = sDefaultFonts;
	mImageMap = sDefaultImages;
	mColorMap = sDefaultColors;
	mSoundMap = sDefaultSounds;
}

unsigned int getHexColor(const char* str, unsigned int defaultColor)
{
	if(!str)
		return defaultColor;

	size_t len = strlen(str);
	if(len != 6 && len != 8)
	{
		LOG(LogError) << "Invalid theme color \"" << str << "\" (must be 6 or 8 characters)";
		return defaultColor;
	}

	unsigned int val;
	std::stringstream ss;
	ss << str;
	ss >> std::hex >> val;

	if(len == 6)
		val = (val << 8) | 0xFF;

	return val;
}

std::string resolvePath(const char* in, const std::string& relative)
{
	if(!in || in[0] == '\0')
		return in;

	boost::filesystem::path relPath(relative);
	relPath = relPath.parent_path();
	
	boost::filesystem::path path(in);
	
	// we use boost filesystem here instead of just string checks because 
	// some directories could theoretically start with ~ or .
	if(*path.begin() == "~")
	{
		path = getHomePath() + (in + 1);
	}else if(*path.begin() == ".")
	{
		path = relPath / (in + 1);
	}

	return path.generic_string();
}

void ThemeData::loadFile(const std::string& themePath)
{
	if(themePath.empty() || !boost::filesystem::exists(themePath))
		return;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(themePath.c_str());
	if(!result)
	{
		LOG(LogWarning) << "Could not parse theme file \"" << themePath << "\":\n   " << result.description();
		return;
	}

	pugi::xml_node root = doc.child("theme");

	// Fonts
	for(auto it = mFontMap.begin(); it != mFontMap.end(); it++)
	{
		pugi::xml_node node = root.child(it->first.c_str());
		if(node)
		{
			std::string path = resolvePath(node.child("path").text().as_string(it->second.path.c_str()), themePath);
			if(!boost::filesystem::exists(path))
			{
				LOG(LogWarning) << "Font \"" << path << "\" doesn't exist!";
				path = it->second.path;
			}

			float size = node.child("size").text().as_float(it->second.size);
			mFontMap[it->first] = FontDef(size, path);
			root.remove_child(node);
		}
	}

	// Images
	for(auto it = mImageMap.begin(); it != mImageMap.end(); it++)
	{
		pugi::xml_node node = root.child(it->first.c_str());
		if(node)
		{
			std::string path = resolvePath(node.child("path").text().as_string(it->second.path.c_str()), themePath);
			if(!boost::filesystem::exists(path))
			{
				LOG(LogWarning) << "Image \"" << path << "\" doesn't exist!";
				path = it->second.path;
			}

			bool tile = node.child("tile").text().as_bool(it->second.tile);
			mImageMap[it->first] = ImageDef(path, tile);
			root.remove_child(node);
		}
	}

	// Colors
	for(auto it = mColorMap.begin(); it != mColorMap.end(); it++)
	{
		pugi::xml_node node = root.child(it->first.c_str());
		if(node)
		{
			mColorMap[it->first] = getHexColor(node.text().as_string(NULL), it->second);
			root.remove_child(node);
		}
	}

	// Sounds
	for(auto it = mSoundMap.begin(); it != mSoundMap.end(); it++)
	{
		pugi::xml_node node = root.child(it->first.c_str());
		if(node)
		{
			std::string path = resolvePath(node.text().as_string(it->second.path.c_str()), themePath);
			if(!boost::filesystem::exists(path))
			{
				LOG(LogWarning) << "Sound \"" << path << "\" doesn't exist!";
				path = it->second.path;
			}

			mSoundMap[it->first] = SoundDef(path);
			root.remove_child(node);
		}
	}

	if(root.begin() != root.end())
	{
		std::stringstream ss;
		ss << "Unused theme identifiers:\n";
		for(auto it = root.children().begin(); it != root.children().end(); it++)
		{
			ss << "    " << it->name() << "\n";
		}

		LOG(LogWarning) << ss.str();
	}
}

void ThemeData::playSound(const std::string& identifier) const
{
	mSoundMap.at(identifier).get()->play();
}
