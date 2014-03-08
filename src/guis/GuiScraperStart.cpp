#include "GuiScraperStart.h"
#include "GuiScraperLog.h"
#include "GuiMsgBoxYesNo.h"

GuiScraperStart::GuiScraperStart(Window* window) : GuiComponent(window),
	mBox(window, ":/frame.png"),
	mList(window, Eigen::Vector2i(2, 4)),
	mFilterLabel(mWindow),
	mSystemsLabel(mWindow),
	mManualLabel(mWindow),
	mFiltersOpt(mWindow),
	mSystemsOpt(mWindow, true),
	mManualSwitch(mWindow),
	mStartButton(mWindow)
{
	mFilterLabel.setText("Filter: ");
	mSystemsLabel.setText("Systems: ");
	mManualLabel.setText("Manual mode: ");

	addChild(&mBox);
	addChild(&mList);

	using namespace Eigen;

	//add filters (with first one selected)
	mFiltersOpt.add("All Games", 
		[](SystemData*, FileData*) -> bool { return true; }, true);
	mFiltersOpt.add("Missing Image", 
		[](SystemData*, FileData* g) -> bool { return g->metadata.get("image").empty(); }, false);

	mList.setEntry(Vector2i(0, 0), Vector2i(1, 1), &mFilterLabel, false, ComponentGrid::AlignRight);
	mList.setEntry(Vector2i(1, 0), Vector2i(1, 1), &mFiltersOpt, true, ComponentGrid::AlignLeft);

	//add systems (all with a platformid specified selected)
	std::vector<SystemData*> sys = SystemData::sSystemVector;
	for(auto it = sys.begin(); it != sys.end(); it++)
		mSystemsOpt.add((*it)->getFullName(), *it, (*it)->getPlatformId() != PlatformIds::PLATFORM_UNKNOWN);

	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), &mSystemsLabel, false, ComponentGrid::AlignRight);
	mList.setEntry(Vector2i(1, 1), Vector2i(1, 1), &mSystemsOpt, true, ComponentGrid::AlignLeft);

	mList.setEntry(Vector2i(0, 2), Vector2i(1, 1), &mManualLabel, false, ComponentGrid::AlignRight);
	mList.setEntry(Vector2i(1, 2), Vector2i(1, 1), &mManualSwitch, true, ComponentGrid::AlignLeft);

	mStartButton.setText("GO GO GO GO", "begin");
	mStartButton.setPressedFunc(std::bind(&GuiScraperStart::pressedStart, this));
	mList.setEntry(Vector2i(0, 3), Vector2i(2, 1), &mStartButton, true, ComponentGrid::AlignCenter);

	mList.setPosition(Renderer::getScreenWidth() / 2 - mList.getSize().x() / 2, Renderer::getScreenHeight() / 2 - mList.getSize().y() / 2);

	mBox.fitTo(mList.getSize(), mList.getPosition(), Eigen::Vector2f(-32, -32));
}

void GuiScraperStart::pressedStart()
{
	std::vector<SystemData*> sys = mSystemsOpt.getSelectedObjects();
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
	std::queue<ScraperSearchParams> searches = getSearches(mSystemsOpt.getSelectedObjects(), mFiltersOpt.getSelectedObjects()[0]);

	GuiScraperLog* gsl = new GuiScraperLog(mWindow, searches, mManualSwitch.getState());
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
	std::vector<HelpPrompt> prompts = mList.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "cancel"));
	return prompts;
}
