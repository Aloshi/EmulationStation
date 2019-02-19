#include "views/gamelist/ISimpleGameListView.h"
#include "ThemeData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Sound.h"
#include "Settings.h"

ISimpleGameListView::ISimpleGameListView(Window* window, const FileData& root) : IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window), mThemeExtras(window)
{
	mHeaderText.setText("Logo Text");
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setAlignment(ALIGN_CENTER);
	
	mHeaderImage.setResize(0, mSize.y() * 0.185f);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);

	mBackground.setResize(mSize.x(), mSize.y());

	addChild(&mHeaderText);
	addChild(&mBackground);
	addChild(&mThemeExtras);

	mCursorStack.push(mRoot);
}

void ISimpleGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	using namespace ThemeFlags;
	mBackground.applyTheme(theme, getName(), "background", ALL);
	mHeaderImage.applyTheme(theme, getName(), "logo", ALL);
	mHeaderText.applyTheme(theme, getName(), "logoText", ALL);
	mThemeExtras.setExtras(ThemeData::makeExtras(theme, getName(), mWindow));

	if(mHeaderImage.hasImage())
	{
		removeChild(&mHeaderText);
		addChild(&mHeaderImage);
	}else{
		addChild(&mHeaderText);
		removeChild(&mHeaderImage);
	}
}

// we could be tricky here to be efficient;
// but this shouldn't happen very often so we'll just always repopulate
void ISimpleGameListView::onFilesChanged()
{
	FileData cursor = getCursor();
	FileData parent = mCursorStack.top();
	populateList(parent.getChildren());
	setCursor(cursor);
}

void ISimpleGameListView::onMetaDataChanged(const FileData& file)
{
	onFilesChanged();
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			FileData cursor = getCursor();
			if(cursor.getType() == GAME)
			{
				Sound::getFromTheme(getTheme(), getName(), "launch")->play();
				launch(cursor);
			}else{
				// it's a folder
				if(cursor.getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor.getChildren());
				}
			}
				
			return true;
		}else if(config->isMappedTo("b", input))
		{
			if(mCursorStack.top() != mRoot)
			{
				FileData old_cursor = mCursorStack.top();
				mCursorStack.pop();
				populateList(mCursorStack.top().getChildren());
				setCursor(old_cursor);
				
				Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}else{
				onFocusLost();
				ViewController::get()->goToSystemView(mRoot.getSystem());
			}

			return true;
		}else if(config->isMappedTo("right", input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToNextGameList();
				return true;
			}
		}else if(config->isMappedTo("left", input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToPrevGameList();
				return true;
			}
		}
	}

	return IGameListView::input(config, input);
}
