#pragma once

#include "../GuiComponent.h"
#include "TextComponent.h"
#include <functional>

class GuiMsgBoxOk : public GuiComponent
{
public:
	GuiMsgBoxOk(Window* window, const std::string& msg, std::function<void()> okCallback = nullptr);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	std::function<void()> mCallback;

	TextComponent mText;
	TextComponent mOkText;
};
