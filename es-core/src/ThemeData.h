#pragma once
#ifndef ES_CORE_THEME_DATA_H
#define ES_CORE_THEME_DATA_H

#include "math/Vector2f.h"
#include <boost/filesystem/path.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include <pugixml/src/pugixml.hpp>
#include <deque>
#include <map>
#include <sstream>

template<typename T>
class TextListComponent;

class GuiComponent;
class ImageComponent;
class NinePatchComponent;
class Sound;
class TextComponent;
class Window;

namespace ThemeFlags
{
	enum PropertyFlags : unsigned int
	{
		PATH = 1,
		POSITION = 2,
		SIZE = 4,
		ORIGIN = 8,
		COLOR = 16,
		FONT_PATH = 32,
		FONT_SIZE = 64,
		SOUND = 128,
		ALIGNMENT = 256,
		TEXT = 512,
		FORCE_UPPERCASE = 1024,
		LINE_SPACING = 2048,
		DELAY = 4096,
		Z_INDEX = 8192,
		ROTATION = 16384,
		ALL = 0xFFFFFFFF
	};
}

class ThemeException : public std::exception
{
public:
	std::string msg;

	virtual const char* what() const throw() { return msg.c_str(); }

	template<typename T>
	friend ThemeException& operator<<(ThemeException& e, T msg);
	
	inline void setFiles(const std::deque<boost::filesystem::path>& deque)
	{
		*this << "from theme \"" << deque.front().string() << "\"\n";
		for(auto it = deque.begin() + 1; it != deque.end(); it++)
			*this << "  (from included file \"" << (*it).string() << "\")\n";
		*this << "    ";
	}
};

template<typename T>
ThemeException& operator<<(ThemeException& e, T appendMsg)
{
	std::stringstream ss;
	ss << e.msg << appendMsg;
	e.msg = ss.str();
	return e;
}

struct ThemeSet
{
	boost::filesystem::path path;

	inline std::string getName() const { return path.stem().string(); }
	inline boost::filesystem::path getThemePath(const std::string& system) const { return path/system/"theme.xml"; }
};

class ThemeData
{
public:

	class ThemeElement
	{
	public:
		bool extra;
		std::string type;

		std::map< std::string, boost::variant<Vector2f, std::string, unsigned int, float, bool> > properties;

		template<typename T>
		T get(const std::string& prop) const { return boost::get<T>(properties.at(prop)); }

		inline bool has(const std::string& prop) const { return (properties.find(prop) != properties.end()); }
	};

private:
	class ThemeView
	{
	public:
		std::map<std::string, ThemeElement> elements;
		std::vector<std::string> orderedKeys;
	};

public:

	ThemeData();

	// throws ThemeException
	void loadFile(std::map<std::string, std::string> sysDataMap, const std::string& path);

	enum ElementPropertyType
	{
		NORMALIZED_PAIR,
		PATH,
		STRING,
		COLOR,
		FLOAT,
		BOOLEAN
	};

	bool hasView(const std::string& view);

	// If expectedType is an empty string, will do no type checking.
	const ThemeElement* getElement(const std::string& view, const std::string& element, const std::string& expectedType) const;

	static std::vector<GuiComponent*> makeExtras(const std::shared_ptr<ThemeData>& theme, const std::string& view, Window* window);

	static const std::shared_ptr<ThemeData>& getDefault();

	static std::map<std::string, ThemeSet> getThemeSets();
	static boost::filesystem::path getThemeFromCurrentSet(const std::string& system);

private:
	static std::map< std::string, std::map<std::string, ElementPropertyType> > sElementMap;
	static std::vector<std::string> sSupportedFeatures;
	static std::vector<std::string> sSupportedViews;

	std::deque<boost::filesystem::path> mPaths;
	float mVersion;

	void parseFeatures(const pugi::xml_node& themeRoot);
	void parseIncludes(const pugi::xml_node& themeRoot);
	void parseVariables(const pugi::xml_node& root);
	void parseViews(const pugi::xml_node& themeRoot);
	void parseView(const pugi::xml_node& viewNode, ThemeView& view);
	void parseElement(const pugi::xml_node& elementNode, const std::map<std::string, ElementPropertyType>& typeMap, ThemeElement& element);

	std::map<std::string, ThemeView> mViews;
};

#endif // ES_CORE_THEME_DATA_H
