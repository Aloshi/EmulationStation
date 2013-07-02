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

	Vector2d getScrollPos() const;
	void setScrollPos(const Vector2d& pos);
	void setAutoScroll(int delay, double speed); //Use 0 for speed to disable.
	void resetAutoScrollTimer();

	void update(int deltaTime) override;
	void onRender() override;

private:
	Font* getFont() const;
	void calculateExtent();

	unsigned int mColor;
	Font* mFont;
	bool mAutoCalcExtent;
	std::string mText;

	//scrolling
	Vector2d mScrollPos;
	Vector2d mScrollDir;
	int mAutoScrollDelay;
	double mAutoScrollSpeed;
	int mAutoScrollTimer;
};

#endif
