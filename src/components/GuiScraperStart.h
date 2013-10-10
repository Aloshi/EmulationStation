#pragma once

#include "../GuiComponent.h"
#include "../SystemData.h"
#include "TextComponent.h"
#include "ComponentListComponent.h"
#include "OptionListComponent.h"
#include "SwitchComponent.h"

class GuiScraperStart : public GuiComponent
{
public:
	GuiScraperStart(Window* window);

private:
	NinePatchComponent mBox;
	ComponentListComponent mList;

	TextComponent mFilterLabel;
	TextComponent mSystemsLabel;
	TextComponent mManualLabel;

	OptionListComponent<unsigned int> mFiltersOpt;
	OptionListComponent<SystemData*> mSystemsOpt;
	SwitchComponent mManualSwitch;
};

/* 

Filter: [MISSING IMAGES | ALL]
Systems: [# selected]
Manual Mode: [ON | OFF]
GO GO GO

*/

