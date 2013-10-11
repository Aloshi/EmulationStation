#pragma once

#include "../GuiComponent.h"
#include "../SystemData.h"
#include "TextComponent.h"
#include "ComponentListComponent.h"
#include "OptionListComponent.h"
#include "SwitchComponent.h"
#include "ButtonComponent.h"
#include "../scrapers/Scraper.h"
#include <queue>

typedef std::function<bool(SystemData*, GameData*)> GameFilterFunc;

class GuiScraperStart : public GuiComponent
{
public:
	GuiScraperStart(Window* window);

	bool input(InputConfig* config, Input input) override;

private:
	void start();
	std::queue<ScraperSearchParams> getSearches(std::vector<SystemData*> systems, GameFilterFunc selector);

	NinePatchComponent mBox;
	ComponentListComponent mList;

	TextComponent mFilterLabel;
	TextComponent mSystemsLabel;
	TextComponent mManualLabel;

	OptionListComponent<GameFilterFunc> mFiltersOpt;
	OptionListComponent<SystemData*> mSystemsOpt;
	SwitchComponent mManualSwitch;

	ButtonComponent mStartButton;
};

/* 

Filter: [MISSING IMAGES | ALL]
Systems: [# selected]
Manual Mode: [ON | OFF]
GO GO GO

*/

