#ifndef _TEXTCOMPONENT_H_
#define _TEXTCOMPONENT_H_

#include "../GuiComponent.h"
#include "../resources/Font.h"

class TextComponent : public GuiComponent
{
public:
	TextComponent(Window* window);
	TextComponent(Window* window, const std::string& text, std::shared_ptr<Font> font, Eigen::Vector3f pos = Eigen::Vector3f::Zero(), Eigen::Vector2f size = Eigen::Vector2f::Zero());

	void setFont(std::shared_ptr<Font> font);
	void onSizeChanged() override;
	void setText(const std::string& text);
	void setColor(unsigned int color);
	void setCentered(bool center); //Default is uncentered.

	void render(const Eigen::Affine3f& parentTrans) override;

	std::string getValue() const override;
	void setValue(const std::string& value) override;

	unsigned char getOpacity() const override;
	void setOpacity(unsigned char opacity) override;
	
	std::shared_ptr<Font> getFont() const;

private:
	void calculateExtent();

	void onTextChanged();
	void onColorChanged();

	unsigned int mColor;
	std::shared_ptr<Font> mFont;
	Eigen::Matrix<bool, 1, 2> mAutoCalcExtent;
	std::string mText;
	std::shared_ptr<TextCache> mTextCache;
	bool mCentered;
};

#endif
