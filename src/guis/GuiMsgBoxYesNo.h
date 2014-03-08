#pragma once

#include "../GuiComponent.h"
#include "../components/TextComponent.h"
#include "../components/NinePatchComponent.h"
#include <functional>

//A simple "yes or no" popup box with callbacks for yes or no.
//Make sure you remember to push it onto the window!
class GuiMsgBoxYesNo : public GuiComponent
{
public:
	GuiMsgBoxYesNo(Window* window, const std::string& msg, std::function<void()> yesCallback = nullptr, std::function<void()> noCallback = nullptr);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& trans) override;

private:
	std::function<void()> mYesCallback, mNoCallback;

	NinePatchComponent mBackground;

	TextComponent mText;
	TextComponent mInputText;
};
