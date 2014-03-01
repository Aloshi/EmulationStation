#pragma once

#include "../GuiComponent.h"
#include "../components/TextComponent.h"
#include <functional>

//A simple popup message box with callbacks for when the user dismisses it.
//Make sure you remember to push it onto the window!
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
