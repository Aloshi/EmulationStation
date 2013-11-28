#pragma once

#include "../GuiComponent.h"
#include "TextListComponent.h"
#include "NinePatchComponent.h"
#include <functional>

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window);

	bool input(InputConfig* config, Input input) override;

private:
	std::shared_ptr<ThemeData> mTheme;
	NinePatchComponent mBackground;
	TextListComponent< std::function<void()> > mList;
};
