#include "views/gamelist/BasicGameListView.h"
#include "views/ViewController.h"
#include "Renderer.h"
#include "Window.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Settings.h"
#include "Locale.h"

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
	ISimpleGameListView::onFileChanged(file, change);

	if(change == FILE_METADATA_CHANGED)
	{
		// might switch to a detailed view
		ViewController::get()->reloadGameListView(this);
		return;
	}

}

void BasicGameListView::populateList(const std::vector<FileData*>& files)
{
	mList.clear();

	const FileData* root = getRoot();
	const SystemData* systemData = root->getSystem();
	mHeaderText.setText(systemData ? systemData->getFullName() : root->getCleanName());

	bool favoritesOnly = false;

	if (Settings::getInstance()->getBool("FavoritesOnly") && !systemData->isFavorite())
	{
		for (auto it = files.begin(); it != files.end(); it++)
		{
			if ((*it)->getType() == GAME)
			{
				if ((*it)->metadata.get("favorite").compare("true") == 0)
				{
					favoritesOnly = true;
					break;
				}
			}
		}
	}

	// The TextListComponent would be able to insert at a specific position,
	// but the cost of this operation could be seriously huge.
	// This naive implemention of doing a first pass in the list is used instead.
	if(!Settings::getInstance()->getBool("FavoritesOnly") || systemData->isFavorite()){
		for(auto it = files.begin(); it != files.end(); it++)
		{
			if ((*it)->getType() != FOLDER &&(*it)->metadata.get("favorite").compare("true") == 0)
			{
				mList.add("\uF006 " + (*it)->getName(), *it, ((*it)->getType() == FOLDER)); // FIXME Folder as favorite ?
			}
		}
	}

	// Do not show double names in favorite system.
	if(!systemData->isFavorite())
	{
		for (auto it = files.begin(); it != files.end(); it++) {
			if (favoritesOnly) {
				if ((*it)->getType() == GAME) {
					if ((*it)->metadata.get("hidden").compare("yes") != 0) {
						if ((*it)->metadata.get("favorite").compare("true") == 0) {
							mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
						}
					}
				}
			}
			else {
				if ((*it)->metadata.get("hidden").compare("true") != 0) {
					if ((*it)->getType() != FOLDER &&(*it)->metadata.get("favorite").compare("true") == 0)
					{
						mList.add("\uF006 " + (*it)->getName(), *it, ((*it)->getType() == FOLDER));
					}else {
						mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
					}
				}
			}
		}
	}
	if(files.size() == 0){
		while(!mCursorStack.empty()){
			mCursorStack.pop();
		}
	}
}

FileData* BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursorIndex(int cursor){
	mList.setCursorIndex(cursor);
}

int BasicGameListView::getCursorIndex(){
	return mList.getCursorIndex();
}

void BasicGameListView::setCursor(FileData* cursor)
{
	if(!mList.setCursor(cursor))
	{
		populateList(mRoot->getChildren());
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
	  prompts.push_back(HelpPrompt("left/right", _("SYSTEM")));
	prompts.push_back(HelpPrompt("up/down", _("CHOOSE")));
	prompts.push_back(HelpPrompt("b", _("LAUNCH")));
	prompts.push_back(HelpPrompt("a", _("BACK")));
	if(getRoot()->getSystem() != SystemData::getFavoriteSystem()) {
	  prompts.push_back(HelpPrompt("y", _("Favorite")));
	  prompts.push_back(HelpPrompt("select", _("OPTIONS")));
	}
	return prompts;
}
