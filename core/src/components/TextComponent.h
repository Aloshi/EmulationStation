#ifndef _TEXTCOMPONENT_H_
#define _TEXTCOMPONENT_H_

#include "GuiComponent.h"
#include "resources/Font.h"

class ThemeData;

// Used to display text.
// TextComponent::setSize(x, y) works a little differently than most components:
//  * (0, 0)                     - will automatically calculate a size that fits the text on one line (expand horizontally)
//  * (x != 0, 0)                - wrap text so that it does not reach beyond x. Will automatically calculate a vertical size (expand vertically).
//  * (x != 0, y <= fontHeight)  - will truncate text so it fits within this box.
class TextComponent : public GuiComponent
{
public:
	TextComponent(Window* window);
	TextComponent(Window* window, const std::string& text, const std::shared_ptr<Font>& font, unsigned int color = 0x000000FF, Alignment align = ALIGN_LEFT,
		Eigen::Vector3f pos = Eigen::Vector3f::Zero(), Eigen::Vector2f size = Eigen::Vector2f::Zero());

	void setFont(const std::shared_ptr<Font>& font);
	void setUppercase(bool uppercase);
	void onSizeChanged() override;
	void setText(const std::string& text);
	void setColor(unsigned int color);
	void setAlignment(Alignment align);
	void setLineSpacing(float spacing);

	void render(const Eigen::Affine3f& parentTrans) override;

	std::string getValue() const override;
	void setValue(const std::string& value) override;

	unsigned char getOpacity() const override;
	void setOpacity(unsigned char opacity) override;
	
	inline std::shared_ptr<Font> getFont() const { return mFont; }

	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

private:
	void calculateExtent();

	void onTextChanged();
	void onColorChanged();

	unsigned int mColor;
	std::shared_ptr<Font> mFont;
	bool mUppercase;
	Eigen::Matrix<bool, 1, 2> mAutoCalcExtent;
	std::string mText;
	std::shared_ptr<TextCache> mTextCache;
	Alignment mAlignment;
	float mLineSpacing;
};

#endif
