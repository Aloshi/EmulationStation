#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <deque>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <Eigen/Dense>
#include "pugixml/pugixml.hpp"
#include "GuiComponent.h"

template<typename T>
class TextListComponent;

class Sound;
class ImageComponent;
class NinePatchComponent;
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

class ThemeExtras : public GuiComponent
{
public:
	ThemeExtras(Window* window) : GuiComponent(window) {};
	virtual ~ThemeExtras();

	// will take ownership of the components within extras (delete them in destructor or when setExtras is called again)
	void setExtras(const std::vector<GuiComponent*>& extras);

private:
	std::vector<GuiComponent*> mExtras;
};

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

		std::map< std::string, boost::variant<Eigen::Vector2f, std::string, unsigned int, float, bool> > properties;

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
	void loadFile(const std::string& path);

	enum ElementPropertyType
	{
		NORMALIZED_PAIR,
		PATH,
		STRING,
		COLOR,
		FLOAT,
		BOOLEAN
	};

	// If expectedType is an empty string, will do no type checking.
	const ThemeElement* getElement(const std::string& view, const std::string& element, const std::string& expectedType) const;

	static std::vector<GuiComponent*> makeExtras(const std::shared_ptr<ThemeData>& theme, const std::string& view, Window* window);

	static const std::shared_ptr<ThemeData>& getDefault();

	static std::map<std::string, ThemeSet> getThemeSets();
	static boost::filesystem::path getThemeFromCurrentSet(const std::string& system);

	bool getHasFavoritesInTheme();

private:
	static std::map< std::string, std::map<std::string, ElementPropertyType> > sElementMap;

	std::deque<boost::filesystem::path> mPaths;
	float mVersion;

	void parseIncludes(const pugi::xml_node& themeRoot);
	void parseViews(const pugi::xml_node& themeRoot);
	void parseView(const pugi::xml_node& viewNode, ThemeView& view);
	void parseElement(const pugi::xml_node& elementNode, const std::map<std::string, ElementPropertyType>& typeMap, ThemeElement& element);

	std::map<std::string, ThemeView> mViews;
};
