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
#include "pugiXML/pugixml.hpp"
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
		TILING = 128,
		SOUND = 256,
		CENTER = 512,
		TEXT = 1024
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
	private:
		bool mExtrasDirty;
		std::vector<GuiComponent*> mExtras;

	public:
		ThemeView() : mExtrasDirty(true) {}
		virtual ~ThemeView();

		std::map<std::string, ThemeElement> elements;
		
		const std::vector<GuiComponent*>& getExtras(Window* window);
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

	void renderExtras(const std::string& view, Window* window, const Eigen::Affine3f& transform);

	void playSound(const std::string& name);

	// If expectedType is an empty string, will do no type checking.
	const ThemeElement* getElement(const std::string& view, const std::string& element, const std::string& expectedType) const;

private:
	static std::map< std::string, std::map<std::string, ElementPropertyType> > sElementMap;

	std::deque<boost::filesystem::path> mPaths;
	float mVersion;

	void parseIncludes(const pugi::xml_node& themeRoot);
	void parseViews(const pugi::xml_node& themeRoot);
	void parseView(const pugi::xml_node& viewNode, ThemeView& view);
	void parseElement(const pugi::xml_node& elementNode, const std::map<std::string, ElementPropertyType>& typeMap, ThemeElement& element);

	std::map<std::string, ThemeView> mViews;

	std::map< std::string, std::shared_ptr<Sound> > mSoundCache;
};


// okay ideas for applying themes to GuiComponents:
// 1. ThemeData::applyToImage(component, args)
//   NO, BECAUSE:
//     - for templated types (TextListComponent) have to include the whole template in a header
//     - inconsistent definitions if mixing templated types (some in a .cpp, some in a .h/.inl)
// 2. template<typename T> ThemeData::apply(component, args) with specialized templates
//   NO, BECAUSE:
//     - doesn't solve the first drawback
//     - can't customize arguments for specific types
// 3. GuiComponent::applyTheme(theme, args)  - WINNER
//   NO, BECAUSE:
//     - can't access private members of ThemeData
//        - can't this be solved with enough getters?
//           - theme->hasElement and theme->getProperty will require 2x as many map lookups (4 vs 2)
//              - why not just return a const ThemeElement...