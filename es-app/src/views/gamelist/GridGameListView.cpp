#include "views/gamelist/GridGameListView.h"

#include "views/ViewController.h"
#include "SystemData.h"

GridGameListView::GridGameListView(Window* window, FileData* root) : ISimpleGameListView(window, root),
	mGrid(window)
{
	mGrid.setPosition(0, mSize.y() * 0.2f);
	mGrid.setSize(mSize.x(), mSize.y() * 0.8f);
	addChild(&mGrid);

	populateList(root->getChildrenListToDisplay());
}

FileData* GridGameListView::getCursor()
{
	return mGrid.getSelected();
}

void GridGameListView::setCursor(FileData* file)
{
	if(!mGrid.setCursor(file))
	{
		populateList(file->getParent()->getChildrenListToDisplay());
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
	for(auto it = files.cbegin(); it != files.cend(); it++)
	{
		mGrid.add((*it)->getName(), (*it)->getThumbnailPath(), *it);
	}
}

void GridGameListView::addPlaceholder()
{
	// empty grid - add a placeholder
	FileData* placeholder = new FileData(PLACEHOLDER, "<No Entries Found>", this->mRoot->getSystem()->getSystemEnvData(), this->mRoot->getSystem());
	mGrid.add(placeholder->getName(), "", placeholder);
}

void GridGameListView::launch(FileData* game)
{
	ViewController::get()->launch(game);
}

void GridGameListView::remove(FileData *game, bool deleteFile)
{
	if (deleteFile)
		Utils::FileSystem::removeFile(game->getPath());  // actually delete the file on the filesystem
	FileData* parent = game->getParent();
	if (getCursor() == game)                     // Select next element in list, or prev if none
	{
		std::vector<FileData*> siblings = parent->getChildrenListToDisplay();
		auto gameIter = std::find(siblings.cbegin(), siblings.cend(), game);
		int gamePos = (int)std::distance(siblings.cbegin(), gameIter);
		if (gameIter != siblings.cend())
		{
			if ((gamePos + 1) < siblings.size())
			{
				setCursor(siblings.at(gamePos + 1));
			} else if ((gamePos - 1) > 0) {
				setCursor(siblings.at(gamePos - 1));
			}
		}
	}
	mGrid.remove(game);
	if(mGrid.size() == 0)
	{
		addPlaceholder();
	}
	delete game;                                 // remove before repopulating (removes from parent)
	onFileChanged(parent, FILE_REMOVED);           // update the view, with game removed
}

std::vector<HelpPrompt> GridGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down/left/right", "scroll"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
