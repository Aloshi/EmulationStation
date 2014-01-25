#include "GridGameListView.h"
#include "../../ThemeData.h"
#include "../../Window.h"
#include "../ViewController.h"

GridGameListView::GridGameListView(Window* window, FileData* root) : ISimpleGameListView(window, root),
	mGrid(window)
{
	mGrid.setPosition(0, mSize.y() * 0.2f);
	mGrid.setSize(mSize.x(), mSize.y() * 0.8f);
	addChild(&mGrid);

	populateList(root->getChildren());
}

FileData* GridGameListView::getCursor()
{
	return mGrid.getSelected();
}

void GridGameListView::setCursor(FileData* file)
{
	typedef ImageGridComponent<FileData*>::Entry Entry;
	auto& list = mGrid.getList();
	auto found = std::find_if(list.begin(), list.end(), [&] (const Entry& e) { return (e.object == file); });
	
	if(found != list.end())
	{
		mGrid.setCursor(found);
	}else{
		populateList(file->getParent()->getChildren());
		mGrid.setCursor(file);
	}
}

bool GridGameListView::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("left", input) || config->isMappedTo("right", input))
		return GuiComponent::input(config, input);

	return ISimpleGameListView::input(config, input);
}

void GridGameListView::populateList(const std::vector<FileData*>& files)
{
	mGrid.clear();
	for(auto it = files.begin(); it != files.end(); it++)
	{
		mGrid.add((*it)->getThumbnailPath(), *it);
	}
}

void GridGameListView::launch(FileData* game)
{
	mWindow->getViewController()->launch(game);
}

std::vector<HelpPrompt> GridGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down/left/right", "scroll"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
