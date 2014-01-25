#pragma once

#include "../GuiComponent.h"
#include <functional>
#include "../resources/Font.h"
#include "NinePatchComponent.h"

class ButtonComponent : public GuiComponent
{
public:
	ButtonComponent(Window* window);

	void setPressedFunc(std::function<void()> f);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void setText(const std::string& text, const std::string& helpText, unsigned int focusedTextColor, unsigned int unfocusedTextColor = 0x555555FF);

	void onSizeChanged() override;
	void onFocusGained() override;
	void onFocusLost() override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	std::shared_ptr<Font> getFont();
	std::function<void()> mPressedFunc;

	bool mFocused;
	unsigned int mTextColorFocused;
	unsigned int mTextColorUnfocused;
	int mTextPulseTime;

	unsigned int getCurTextColor() const;

	std::string mText;
	std::string mHelpText;
	std::unique_ptr<TextCache> mTextCache;
	NinePatchComponent mBox;
};
