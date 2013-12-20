#include "GridGameListView.h"
#include "../ThemeData.h"
#include "../Window.h"
#include "ViewController.h"

GridGameListView::GridGameListView(Window* window, FileData* root) : GameListView(window, root),
	mGrid(window), mBackground(window)
{
	mBackground.setResize(mSize.x(), mSize.y(), true);
	addChild(&mBackground);

	mGrid.setPosition(0, mSize.y() * 0.2f);
	mGrid.setSize(mSize.x(), mSize.y() * 0.8f);
	addChild(&mGrid);

	populateList(root);
}

void GridGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	// fuck it, just completely repopulate always all the time
	FileData* cursor = getCursor();
	populateList(cursor->getParent());
	mGrid.setCursor(cursor);
}

void GridGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	mBackground.setImage(theme->getImage("backgroundImage").getTexture());
	mBackground.setTiling(theme->getImage("backgroundImage").tile);
}

FileData* GridGameListView::getCursor()
{
	return mGrid.getSelected();
}

void GridGameListView::setCursor(FileData* file)
{
	assert(file->getParent() == getCursor()->getParent()); // too lazy to implement right now
	mGrid.setCursor(file);
}

bool GridGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			if(mGrid.getList().size() > 0)
			{
				FileData* cursor = getCursor();
				if(cursor->getType() == GAME)
				{
					mWindow->getViewController()->launch(cursor);
				}else{
					// it's a folder
					if(cursor->getChildren().size() > 0)
					{
						mCursorStack.push(cursor);
						populateList(cursor);
					}
				}
				
				return true;
			}
		}else if(config->isMappedTo("b", input))
		{
			if(mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent());
				mGrid.setCursor(mCursorStack.top());
				mCursorStack.pop();
				mTheme->playSound("backSound");
			}else{
				mGrid.stopScrolling();
				mWindow->getViewController()->goToSystemSelect();
			}

			return true;
		}
	}
	return GameListView::input(config, input);
}

void GridGameListView::populateList(FileData* root)
{
	mGrid.clear();
	for(auto it = root->getChildren().begin(); it != root->getChildren().end(); it++)
	{
		mGrid.add((*it)->getThumbnailPath(), *it);
	}
}
