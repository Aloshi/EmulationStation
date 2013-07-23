#ifndef _TEXTCOMPONENT_H_
#define _TEXTCOMPONENT_H_

#include "../GuiComponent.h"
#include "../Font.h"

class TextComponent : public GuiComponent
{
public:
	TextComponent(Window* window);
	TextComponent(Window* window, const std::string& text, std::shared_ptr<Font> font, Eigen::Vector3f pos, Eigen::Vector2f size);

	void setFont(std::shared_ptr<Font> font);
	void onSizeChanged() override;
	void setText(const std::string& text);
	void setColor(unsigned int color);
	void setCentered(bool center); //Default is uncentered.

	void render(const Eigen::Affine3f& parentTrans) override;

private:
	std::shared_ptr<Font> getFont() const;
	
	void calculateExtent();

	unsigned int mColor;
	std::shared_ptr<Font> mFont;
	Eigen::Matrix<bool, 1, 2> mAutoCalcExtent;
	std::string mText;
	bool mCentered;
};

#endif
