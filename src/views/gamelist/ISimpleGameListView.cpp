#include "ISimpleGameListView.h"
#include "../../ThemeData.h"
#include "../../Window.h"
#include "../ViewController.h"

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) : IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window)
{
	mHeaderText.setText("Header");
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setCentered(true);
	
	mHeaderImage.setResize(0, mSize.y() * 0.185f, false);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);

	mBackground.setResize(mSize.x(), mSize.y(), true);

	addChild(&mHeaderText);
	addChild(&mBackground);
}

void ISimpleGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
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
}

void ISimpleGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	// we could be tricky here to be efficient;
	// but this shouldn't happen very often so we'll just always repopulate
	FileData* cursor = getCursor();
	populateList(cursor->getParent()->getChildren());
	setCursor(cursor);
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			FileData* cursor = getCursor();
			if(cursor->getType() == GAME)
			{
				launch(cursor);
			}else{
				// it's a folder
				if(cursor->getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor->getChildren());
				}
			}
				
			return true;
		}else if(config->isMappedTo("b", input))
		{
			if(mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent()->getChildren());
				setCursor(mCursorStack.top());
				mCursorStack.pop();
				getTheme()->playSound("backSound");
			}else{
				onFocusLost();
				mWindow->getViewController()->goToSystemSelect();
			}

			return true;
		}else if(config->isMappedTo("right", input))
		{
			onFocusLost();
			mWindow->getViewController()->goToNextGameList();
			return true;
		}else if(config->isMappedTo("left", input))
		{
			onFocusLost();
			mWindow->getViewController()->goToPrevGameList();
			return true;
		}
	}

	return IGameListView::input(config, input);
}
