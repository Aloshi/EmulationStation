#pragma once

#include "../GuiComponent.h"
#include <functional>
#include "../resources/Font.h"
#include "NinePatchComponent.h"

class ButtonComponent : public GuiComponent
{
public:
	ButtonComponent(Window* window, const std::string& text = "", const std::string& helpText = "", const std::function<void()>& func = nullptr);

	void setPressedFunc(std::function<void()> f);

	void setEnabled(bool enable);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void setText(const std::string& text, const std::string& helpText);

	void onSizeChanged() override;
	void onFocusGained() override;
	void onFocusLost() override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	std::shared_ptr<Font> getFont();
	std::function<void()> mPressedFunc;

	bool mFocused;
	bool mEnabled;
	unsigned int mTextColorFocused;
	unsigned int mTextColorUnfocused;
	
	unsigned int getCurTextColor() const;
	void updateImage();

	std::string mText;
	std::string mHelpText;
	std::unique_ptr<TextCache> mTextCache;
	NinePatchComponent mBox;
};
