#include "GuiScraperStart.h"
#include "GuiScraperLog.h"
#include "GuiMsgBoxYesNo.h"

#include "../components/TextComponent.h"
#include "../components/OptionListComponent.h"
#include "../components/SwitchComponent.h"

GuiScraperStart::GuiScraperStart(Window* window) : GuiComponent(window),
	mMenu(window, "SCRAPE NOW")
{
	addChild(&mMenu);

	// add filters (with first one selected)
	mFilters = std::make_shared< OptionListComponent<GameFilterFunc> >(mWindow, false);
	mFilters->add("All Games", 
		[](SystemData*, FileData*) -> bool { return true; }, true);
	mFilters->add("Only missing image", 
		[](SystemData*, FileData* g) -> bool { return g->metadata.get("image").empty(); }, false);
	mMenu.addWithLabel("Filter", mFilters);

	//add systems (all with a platformid specified selected)
	mSystems = std::make_shared< OptionListComponent<SystemData*> >(mWindow, true);
	for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); it++)
		mSystems->add((*it)->getFullName(), *it, (*it)->getPlatformId() != PlatformIds::PLATFORM_UNKNOWN);
	mMenu.addWithLabel("Systems", mSystems);

	mAutoStyle = std::make_shared< OptionListComponent<int> >(mWindow, false);
	mAutoStyle->add("Never automatically accept result", 0, true);
	mAutoStyle->add("Always accept first result", 1, false);
	mMenu.addWithLabel("Auto style", mAutoStyle);

	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, "GO GO GO", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.makeAcceptInputHandler(std::bind(&GuiScraperStart::pressedStart, this));
	mMenu.addRow(row);

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
}

void GuiScraperStart::pressedStart()
{
	std::vector<SystemData*> sys = mSystems->getSelectedObjects();
	for(auto it = sys.begin(); it != sys.end(); it++)
	{
		if((*it)->getPlatformId() == PlatformIds::PLATFORM_UNKNOWN)
		{
			mWindow->pushGui(new GuiMsgBoxYesNo(mWindow, "Warning: some of your selected systems do not have a platform ID set. Results may be even more inaccurate than usual!\nContinue anyway?", 
				std::bind(&GuiScraperStart::start, this)));
			return;
		}
	}

	start();
}

void GuiScraperStart::start()
{
	std::queue<ScraperSearchParams> searches = getSearches(mSystems->getSelectedObjects(), mFilters->getSelected());

	GuiScraperLog* gsl = new GuiScraperLog(mWindow, searches, mAutoStyle->getSelected() == 0);
	mWindow->pushGui(gsl);
	gsl->start();
	delete this;
}

std::queue<ScraperSearchParams> GuiScraperStart::getSearches(std::vector<SystemData*> systems, GameFilterFunc selector)
{
	std::queue<ScraperSearchParams> queue;
	for(auto sys = systems.begin(); sys != systems.end(); sys++)
	{
		std::vector<FileData*> games = (*sys)->getRootFolder()->getFilesRecursive(GAME);
		for(auto game = games.begin(); game != games.end(); game++)
		{
			if(selector((*sys), (*game)))
			{
				ScraperSearchParams search;
				search.game = *game;
				search.system = *sys;
				
				queue.push(search);
			}
		}
	}

	return queue;
}

bool GuiScraperStart::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;
	
	if(input.value != 0 && config->isMappedTo("b", input))
	{
		delete this;
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiScraperStart::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "cancel"));
	return prompts;
}
