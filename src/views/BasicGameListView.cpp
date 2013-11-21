#include "BasicGameListView.h"
#include "ViewController.h"
#include "../Renderer.h"
#include "../Window.h"
#include "../ThemeData.h"

BasicGameListView::BasicGameListView(Window* window, FileData* root)
	: GameListView(window, root), 
	mHeaderText(window), mHeaderImage(window), mBackground(window), mList(window)
{
	mHeaderText.setText("Header");
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setCentered(true);
	
	mHeaderImage.setResize(0, mSize.y() * 0.185f, false);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);

	mBackground.setResize(mSize.x(), mSize.y(), true);

	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);

	populateList(root);

	addChild(&mBackground);
	addChild(&mList);
	addChild(&mHeaderText);
}

void BasicGameListView::setTheme(const std::shared_ptr<ThemeData>& theme)
{
	const ImageDef& bg = theme->getImage("backgroundImage");
	mBackground.setTiling(bg.tile);
	mBackground.setImage(bg.getTexture());
	
	const ImageDef& hdr = theme->getImage("headerImage");
	mHeaderImage.setTiling(hdr.tile);
	mHeaderImage.setImage(hdr.getTexture());
	
	if(mHeaderImage.hasImage())
	{
		removeChild(&mHeaderText);
		addChild(&mHeaderImage);
	}else{
		addChild(&mHeaderText);
		removeChild(&mHeaderImage);
	}

	mList.setTheme(theme);
}

void BasicGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	// we don't care about metadata changes (since we don't display metadata), 
	// so we just ignore the FILE_METADATA_CHANGED case

	// if it's immediately inside our current folder
	if(file->getParent() == getCurrentFolder())
	{
		if(change == FILE_REMOVED)
		{
			mList.remove(file); // will automatically make sure cursor ends up in a "safe" place
		}else if(change == FILE_ADDED)
		{
			FileData* cursor = getCursor();
			populateList(cursor->getParent());
			mList.setCursor(cursor);
		}
	}
}

void buildHeader(FileData* from, std::stringstream& ss)
{
	if(from->getParent())
	{
		buildHeader(from->getParent(), ss);
		ss << " -> ";
	}

	ss << from->getName();
}

void BasicGameListView::populateList(FileData* root)
{
	mList.clear();

	std::stringstream ss;
	buildHeader(root, ss);
	mHeaderText.setText(ss.str());

	for(auto it = root->getChildren().begin(); it != root->getChildren().end(); it++)
	{
		mList.add((*it)->getName(), *it, ((*it)->getType() == FOLDER));
	}
}

void BasicGameListView::setCursor(FileData* cursor)
{
	if(cursor->getParent() != getCursor()->getParent())
	{
		// Rebuild the folder stack
		std::stack<FileData*> path;
		FileData* cur = cursor;
		while((cur = cur->getParent()) != mRoot)
			path.push(cur);

		while(!mCursorStack.empty())
			mCursorStack.pop();

		while(!path.empty()) // put back in reverse order (flip)
		{
			mCursorStack.push(path.top());
			path.pop();
		}

		populateList(cursor->getParent());
	}

	mList.setCursor(cursor);
}

bool BasicGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			if(mList.getList().size() > 0)
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
				mList.setCursor(mCursorStack.top());
				mCursorStack.pop();
			}else{
				mList.stopScrolling();
				mWindow->getViewController()->goToSystemSelect();
			}

			return true;
		}else if(config->isMappedTo("right", input))
		{
			mList.stopScrolling();
			mWindow->getViewController()->goToNextSystem();
			return true;
		}else if(config->isMappedTo("left", input))
		{
			mList.stopScrolling();
			mWindow->getViewController()->goToPrevSystem();
			return true;
		}
	}

	return GameListView::input(config, input);
}
