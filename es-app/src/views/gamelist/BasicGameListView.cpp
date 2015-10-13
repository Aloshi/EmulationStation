#include "views/gamelist/BasicGameListView.h"
#include "views/ViewController.h"
#include "Renderer.h"
#include "Window.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Settings.h"

BasicGameListView::BasicGameListView(Window* window, FileData* root)
	: ISimpleGameListView(window, root), mList(window)
{
	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);
	addChild(&mList);

	populateList(root->getChildren());
}

void BasicGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	ISimpleGameListView::onThemeChanged(theme);
	using namespace ThemeFlags;
	mList.applyTheme(theme, getName(), "gamelist", ALL);
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

	const FileData* root = getRoot();
	const SystemData* systemData = root->getSystem();
	mHeaderText.setText(systemData ? systemData->getFullName() : root->getCleanName());

	bool hasFavorites = false;

	if (Settings::getInstance()->getBool("FavoritesOnly"))
	{
		for (auto it = files.begin(); it != files.end(); it++)
		{
			if ((*it)->getType() == GAME)
			{
				if ((*it)->metadata.get("favorite").compare("yes") == 0)
				{
					hasFavorites = true;
					break;
				}
			}
		}
	}

	// The TextListComponent would be able to insert at a specific position,
	// but the cost of this operation could be seriously huge.
	// This naive implemention of doing a first pass in the list is used instead.
	if(! Settings::getInstance()->getBool("FavoritesOnly")){
		for(auto it = files.begin(); it != files.end(); it++)
		{
			if ((*it)->getType() != FOLDER &&(*it)->metadata.get("favorite").compare("yes") == 0)
			{
				mList.add("\uF006 " + (*it)->getName(), *it, ((*it)->getType() == FOLDER)); // FIXME Folder as favorite ?
			}
		}
	}

	for(auto it = files.begin(); it != files.end(); it++)
	{
		if (Settings::getInstance()->getBool("FavoritesOnly") && hasFavorites)
		{
			if ((*it)->getType() == GAME)
			{
				if ((*it)->metadata.get("favorite").compare("yes") == 0)
				{
					mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
				}
			}
		}
		else
		{
			mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
		}
	}
}

FileData* BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursor(FileData* cursor)
{
	if(!mList.setCursor(cursor))
	{
		populateList(cursor->getParent()->getChildren());
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

void BasicGameListView::launch(FileData* game)
{
	ViewController::get()->launch(game);
}

std::vector<HelpPrompt> BasicGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if(Settings::getInstance()->getBool("QuickSystemSelect"))
		prompts.push_back(HelpPrompt("left/right", "system"));
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("y", "favorite"));
	prompts.push_back(HelpPrompt("select", "options"));
	return prompts;
}
