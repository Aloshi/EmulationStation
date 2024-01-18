#include "views/gamelist/BasicGameListView.h"

#include "utils/FileSystemUtil.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "Settings.h"
#include "SystemData.h"

BasicGameListView::BasicGameListView(Window* window, FileData* root)
	: ISimpleGameListView(window, root), mList(window)
{
	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);
	mList.setDefaultZIndex(20);
	addChild(&mList);

	populateList(root->getChildrenListToDisplay());
}

void BasicGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	ISimpleGameListView::onThemeChanged(theme);
	using namespace ThemeFlags;
	mList.applyTheme(theme, getName(), "gamelist", ALL);

	sortChildren();
}

void BasicGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	if(change == FILE_METADATA_CHANGED)
	{
		// might switch to a detailed view
		ViewController::get()->reloadGameListView(this);
		return;
	}

	ISimpleGameListView::onFileChanged(file, change);
}

void BasicGameListView::populateList(const std::vector<FileData*>& files)
{
	mList.clear();
	mHeaderText.setText(mRoot->getSystem()->getFullName());
	if (files.size() > 0)
	{
		for(auto it = files.cbegin(); it != files.cend(); it++)
		{
			mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
		}
	}
	else
	{
		addPlaceholder();
	}
}

FileData* BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursor(FileData* cursor)
{
	if(!mList.setCursor(cursor) && (!cursor->isPlaceHolder()))
	{
		populateList(cursor->getParent()->getChildrenListToDisplay());
		mList.setCursor(cursor);

		// update our cursor stack in case our cursor just got set to some folder we weren't in before
		if(mCursorStack.empty() || mCursorStack.top() != cursor->getParent())
		{
			std::stack<FileData*> tmp;
			FileData* ptr = cursor->getParent();
			while(ptr && ptr != mRoot)
			{
				tmp.push(ptr);
				ptr = ptr->getParent();
			}

			// flip the stack and put it in mCursorStack
			mCursorStack = std::stack<FileData*>();
			while(!tmp.empty())
			{
				mCursorStack.push(tmp.top());
				tmp.pop();
			}
		}
	}
}

void BasicGameListView::setViewportTop(int index)
{
	mList.setViewportTop(index);
}


int BasicGameListView::getViewportTop()
{
	return mList.getViewportTop();
}

void BasicGameListView::addPlaceholder()
{
	// empty list - add a placeholder
	FileData* placeholder = new FileData(PLACEHOLDER, "<No Entries Found>", this->mRoot->getSystem()->getSystemEnvData(), this->mRoot->getSystem());
	mList.add(placeholder->getName(), placeholder, (placeholder->getType() == PLACEHOLDER));
}

std::string BasicGameListView::getQuickSystemSelectRightButton()
{
	return "right";
}

std::string BasicGameListView::getQuickSystemSelectLeftButton()
{
	return "left";
}

void BasicGameListView::launch(FileData* game)
{
	ViewController::get()->launch(game);
}

void BasicGameListView::remove(FileData *game, bool deleteFile, bool refreshView)
{
	if (deleteFile)
	{
		Utils::FileSystem::removeFile(game->getPath());  // actually delete the file on the filesystem

		// we want to delete related/scraped files, but check first if resources are shared with another game
		bool keepVideo     = game->getVideoPath().empty();
		bool keepMarquee   = game->getMarqueePath().empty();
		bool keepThumbnail = game->getThumbnailPath().empty();
		bool keepImage     = game->getImagePath().empty();

		for (auto system : SystemData::sSystemVector)
		{
			// skip checking if we determined we need to keep the resources
			if (keepVideo && keepMarquee && keepImage && keepThumbnail)
				break;

			if (!system->isGameSystem() || system->isCollection())
				continue;

			for (auto entry : system->getRootFolder()->getChildren()) {
				if (entry == game) // skip the game's own entry
					continue;

				if (!keepVideo && (game->getVideoPath() == entry->getVideoPath()))
					keepVideo = true;

				if (!keepMarquee && (game->getMarqueePath() == entry->getMarqueePath()))
					keepMarquee = true;

				// Thumbnail/Image can be used inter-changeably, so check for both in game's resources
				if (!keepThumbnail && (game->getThumbnailPath() == entry->getThumbnailPath() || game->getThumbnailPath() == entry->getImagePath()))
					keepThumbnail = true;

				if (!keepImage && (game->getImagePath() == entry->getImagePath() || game->getImagePath() == entry->getThumbnailPath()))
					keepImage = true;
			}
		}

		// delete the resources that are not shared
		if (!keepVideo)
			Utils::FileSystem::removeFile(game->getVideoPath());
		if (!keepImage)
			Utils::FileSystem::removeFile(game->getImagePath());
		if (!keepThumbnail)
			Utils::FileSystem::removeFile(game->getThumbnailPath());
		if (!keepMarquee)
			Utils::FileSystem::removeFile(game->getMarqueePath());
	}
	FileData* parent = game->getParent();
	if (getCursor() == game)                     // Select next element in list, or prev if none
	{
		std::vector<FileData*> siblings = parent->getChildrenListToDisplay();
		auto gameIter = std::find(siblings.cbegin(), siblings.cend(), game);
		unsigned int gamePos = (int)std::distance(siblings.cbegin(), gameIter);
		if (gameIter != siblings.cend())
		{
			if ((gamePos + 1) < siblings.size())
			{
				setCursor(siblings.at(gamePos + 1));
			} else if (gamePos > 1) {
				setCursor(siblings.at(gamePos - 1));
			}
		}
	}
	mList.remove(game);
	if(mList.size() == 0)
	{
		addPlaceholder();
	}
	delete game;                                 // remove before repopulating (removes from parent)
	if(refreshView)
		onFileChanged(parent, FILE_REMOVED);     // update the view, with game removed
}

std::vector<HelpPrompt> BasicGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if(Settings::getInstance()->getBool("QuickSystemSelect"))
		prompts.push_back(HelpPrompt("left/right", "system"));
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	if(!UIModeController::getInstance()->isUIModeKid())
		prompts.push_back(HelpPrompt("select", "options"));
	if(mRoot->getSystem()->isGameSystem())
		prompts.push_back(HelpPrompt("x", "random"));
	if(mRoot->getSystem()->isGameSystem() && !UIModeController::getInstance()->isUIModeKid())
	{
		std::string prompt = CollectionSystemManager::get()->getEditingCollection();
		prompts.push_back(HelpPrompt("y", prompt));
	}
	return prompts;
}

void BasicGameListView::onFocusLost() {
	mList.stopScrolling(true);
}
