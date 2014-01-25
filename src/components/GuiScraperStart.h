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

typedef std::function<bool(SystemData*, FileData*)> GameFilterFunc;

//The starting point for a multi-game scrape.
//Allows the user to set various parameters (to set filters, to set which systems to scrape, to enable manual mode).
//Generates a list of "searches" that will be carried out by GuiScraperLog.
class GuiScraperStart : public GuiComponent
{
public:
	GuiScraperStart(Window* window);

	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void pressedStart();
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

