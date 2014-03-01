#pragma once

#include "../GuiComponent.h"
#include "../components/TextComponent.h"
#include <functional>

//A simple "yes or no" popup box with callbacks for yes or no.
//Make sure you remember to push it onto the window!
class GuiMsgBoxYesNo : public GuiComponent
{
public:
	GuiMsgBoxYesNo(Window* window, const std::string& msg, std::function<void()> yesCallback = nullptr, std::function<void()> noCallback = nullptr);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	std::function<void()> mYesCallback, mNoCallback;

	TextComponent mText;
	TextComponent mInputText;
};
