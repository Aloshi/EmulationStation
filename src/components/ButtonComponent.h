#pragma once

#include "../GuiComponent.h"
#include <functional>
#include "../Font.h"
#include "NinePatchComponent.h"

class ButtonComponent : public GuiComponent
{
public:
	ButtonComponent(Window* window);

	void setPressedFunc(std::function<void()> f);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void setText(const std::string& text, unsigned int color);

	void onSizeChanged() override;

private:
	std::shared_ptr<Font> getFont();
	std::function<void()> mPressedFunc;

	std::string mText;
	std::unique_ptr<TextCache> mTextCache;
	NinePatchComponent mBox;
};
