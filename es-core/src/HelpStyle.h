#pragma once

#include "math/Vector2f.h"
#include <memory>
#include <string>

class ThemeData;
class Font;

struct HelpStyle
{
	Vector2f position;
	unsigned int iconColor;
	unsigned int textColor;
	std::shared_ptr<Font> font;

	HelpStyle(); // default values
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view);
};