#ifndef _TEXTCOMPONENT_H_
#define _TEXTCOMPONENT_H_

#include "../GuiComponent.h"
#include "../Font.h"

class TextComponent : public GuiComponent
{
public:
	TextComponent(Window* window);
	TextComponent(Window* window, const std::string& text, std::shared_ptr<Font> font, Vector2i pos, Vector2u size);

	void setFont(std::shared_ptr<Font> font);
	void setBox(Vector2i pos, Vector2u size);
	void setExtent(Vector2u size); //Use Vector2u(0, 0) to automatically generate extent on a single line.  Use Vector2(value, 0) to automatically generate extent for wrapped text.
	void setText(const std::string& text);
	void setColor(unsigned int color);

	void onRender() override;

private:
	std::shared_ptr<Font> getFont() const;
	
	void calculateExtent();

	unsigned int mColor;
	std::shared_ptr<Font> mFont;
	Vector2<bool> mAutoCalcExtent;
	std::string mText;
};

#endif
