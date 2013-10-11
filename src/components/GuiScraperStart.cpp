#include "GuiScraperStart.h"

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
	mFiltersOpt.addEntry(mFiltersOpt.makeEntry("All Games", 
		[](SystemData*, GameData*) -> bool { return true; }, true));
	mFiltersOpt.addEntry(mFiltersOpt.makeEntry("Missing Image", 
		[](SystemData*, GameData* g) -> bool { return g->metadata()->get("image").empty(); }));

	mList.setEntry(Vector2i(0, 0), Vector2i(1, 1), &mFilterLabel, false, ComponentListComponent::AlignRight);
	mList.setEntry(Vector2i(1, 0), Vector2i(1, 1), &mFiltersOpt, true, ComponentListComponent::AlignLeft);

	//add systems (with all selected)
	std::vector<SystemData*> sys = SystemData::sSystemVector;
	mSystemsOpt.populate(sys, 
		[&](SystemData* s) { 
			return mSystemsOpt.makeEntry(s->getName(), s, true); 
	});

	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), &mSystemsLabel, false, ComponentListComponent::AlignRight);
	mList.setEntry(Vector2i(1, 1), Vector2i(1, 1), &mSystemsOpt, true, ComponentListComponent::AlignLeft);

	mList.setEntry(Vector2i(0, 2), Vector2i(1, 1), &mManualLabel, false, ComponentListComponent::AlignRight);
	mList.setEntry(Vector2i(1, 2), Vector2i(1, 1), &mManualSwitch, true, ComponentListComponent::AlignLeft);

	mStartButton.setText("GO GO GO GO", 0x00FF00FF);
	mStartButton.setPressedFunc(std::bind(&GuiScraperStart::start, this));
	mList.setEntry(Vector2i(0, 3), Vector2i(2, 1), &mStartButton, true, ComponentListComponent::AlignCenter);

	mList.setPosition(Renderer::getScreenWidth() / 2 - mList.getSize().x() / 2, Renderer::getScreenHeight() / 2 - mList.getSize().y() / 2);

	mBox.setEdgeColor(0x333333FF);
	mBox.fitTo(mList.getSize(), mList.getPosition(), Eigen::Vector2f(8, 8));
}

void GuiScraperStart::start()
{
	std::queue<ScraperSearchParams> searches = getSearches(mSystemsOpt.getSelectedObjects(), mFiltersOpt.getSelectedObjects()[0]);
}

std::queue<ScraperSearchParams> GuiScraperStart::getSearches(std::vector<SystemData*> systems, GameFilterFunc selector)
{
	std::queue<ScraperSearchParams> queue;
	for(auto sys = systems.begin(); sys != systems.end(); sys++)
	{
		std::vector<FileData*> games = (*sys)->getRootFolder()->getFilesRecursive(true);
		for(auto game = games.begin(); game != games.end(); game++)
		{
			if(selector((*sys), (GameData*)(*game)))
			{
				ScraperSearchParams search;
				search.game = (GameData*)(*game);
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
