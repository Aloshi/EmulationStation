#pragma once

#include <Eigen/Dense>
#include <memory>
#include <string>

class ThemeData;
class Font;

struct HelpStyle
{
	Eigen::Vector2f position;
	unsigned int iconColor;
	unsigned int textColor;
	std::shared_ptr<Font> font;

	HelpStyle(); // default values
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view);
};