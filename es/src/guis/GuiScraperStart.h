#pragma once

#include "GuiComponent.h"
#include "SystemData.h"
#include "scrapers/Scraper.h"
#include "components/MenuComponent.h"
#include <queue>

typedef std::function<bool(SystemData*, FileData*)> GameFilterFunc;

template<typename T>
class OptionListComponent;

class SwitchComponent;

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

	std::shared_ptr< OptionListComponent<GameFilterFunc> > mFilters;
	std::shared_ptr< OptionListComponent<SystemData*> > mSystems;
	std::shared_ptr<SwitchComponent> mApproveResults;

	MenuComponent mMenu;
};
