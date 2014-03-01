#pragma once

#include "../GuiComponent.h"
#include "../components/MenuComponent.h"
#include <functional>

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window);

	bool input(InputConfig* config, Input input) override;

private:
	MenuComponent mMenu;
};
