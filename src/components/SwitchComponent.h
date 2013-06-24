#pragma once

#include "../GuiComponent.h"

class SwitchComponent : public GuiComponent
{
public:
	SwitchComponent(Window* window, bool state = false);

	bool input(InputConfig* config, Input input) override;
	void onRender() override;

	bool getState();
	void setState(bool state);

private:
	bool mState;
};
