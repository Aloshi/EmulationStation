#ifndef _TEXTCOMPONENT_H_
#define _TEXTCOMPONENT_H_

#include "../GuiComponent.h"
#include "../Font.h"

class TextComponent : public GuiComponent
{
public:
	TextComponent(Window* window);
	TextComponent(Window* window, const std::string& text, Font* font, Vector2i pos, Vector2u size);

	void setFont(Font* font);
	void setBox(Vector2i pos, Vector2u size);
	void setExtent(Vector2u size); //Use Vector2u(0, 0) to automatically generate extent.
	void setText(const std::string& text);
	void setColor(unsigned int color);

	void onRender();

private:
	void calculateExtent();

	unsigned int mColor;
	Font* mFont;
	bool mAutoCalcExtent;
	std::string mText;
};

#endif
