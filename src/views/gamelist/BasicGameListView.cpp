#include "BasicGameListView.h"
#include "../ViewController.h"
#include "../../Renderer.h"
#include "../../Window.h"
#include "../../ThemeData.h"
#include "../../SystemData.h"

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
	mList.applyTheme(theme, getName(), "gamelist", POSITION | ThemeFlags::SIZE | COLOR | SOUND | FONT_PATH | FONT_SIZE);
}

void BasicGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	if(change == FILE_METADATA_CHANGED)
	{
		// might switch to a detailed view
		mWindow->getViewController()->reloadGameListView(this);
		return;
	}

	ISimpleGameListView::onFileChanged(file, change);
}

void BasicGameListView::populateList(const std::vector<FileData*>& files)
{
	mList.clear();

	mHeaderText.setText(files.at(0)->getSystem()->getFullName());

	for(auto it = files.begin(); it != files.end(); it++)
	{
		mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
	}
}

FileData* BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursor(FileData* cursor)
{
	typedef TextListComponent<FileData*>::ListRow Row;
	const std::vector<Row>& list = mList.getList();
	auto found = std::find_if(list.begin(), list.end(), [&](const Row& row) { return (row.object == cursor); });

	if(found != list.end())
	{
		mList.setCursor(found);
	}else{
		populateList(cursor->getParent()->getChildren());
		mList.setCursor(cursor);
	}
}

void BasicGameListView::launch(FileData* game)
{
	mWindow->getViewController()->launch(game);
}
