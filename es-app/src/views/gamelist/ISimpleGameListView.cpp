#include "views/gamelist/ISimpleGameListView.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Sound.h"
#include "Settings.h"
#include "Gamelist.h"
#include "Log.h"

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) : IGameListView(window, root),
mHeaderText(window), mHeaderImage(window), mBackground(window), mThemeExtras(window), mFavoriteChange(false), mKidGameChange(false)
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

void ISimpleGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	// we could be tricky here to be efficient;
	// but this shouldn't happen very often so we'll just always repopulate
	FileData* cursor = getCursor();
	populateList(cursor->getParent()->getChildren(true));
	setCursor(cursor);
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			LOG(LogDebug) << "ISimpleGameListView::input(): a detected!";
			FileData* cursor = getCursor();
			if(cursor->getType() == GAME)
			{
				Sound::getFromTheme(getTheme(), getName(), "launch")->play();
				launch(cursor);
			}else{
				// it's a folder
				if(cursor->getChildren(true).size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor->getChildren(true));
				}
			}
				
			return true;
		}else if(config->isMappedTo("b", input))
		{
			LOG(LogDebug) << "ISimpleGameListView::input(): b detected!";
			if(mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent()->getChildren(true));
				setCursor(mCursorStack.top());
				mCursorStack.pop();
				Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}else{
				onFocusLost();
				if (mFavoriteChange || mKidGameChange)
				{
					ViewController::get()->setInvalidGamesList(getCursor()->getSystem());
					mFavoriteChange = false;
					mKidGameChange = false;
				}
				ViewController::get()->goToSystemView(getCursor()->getSystem());
			}

			return true;
		}else if (config->isMappedTo("x", input))
		{
			FileData* cursor = getCursor();
			LOG(LogDebug) << "ISimpleGameListView::input(): x detected!";
			if (cursor->getSystem()->getHasFavorites())
			{
				if (cursor->getType() == GAME)
				{
					mFavoriteChange = true;
					MetaDataList* md = &cursor->metadata;
					std::string value = md->get("favorite");
					LOG(LogDebug) << "Favorite = "<< value;
					if (value.compare("false") == 0)
					{
						md->set("favorite", "true");
					}else
					{
						md->set("favorite", "false");
					}
					LOG(LogDebug) << "New Favorite value set to: "<< md->get("favorite");
					updateInfoPanel();
				}
			}
		}else if (config->isMappedTo("y", input))
		{
			LOG(LogDebug) << "ISimpleGameListView::input(): y detected!";
			FileData* cursor = getCursor();
			if (cursor->getSystem()->getHasKidGames() && 
				Settings::getInstance()->getString("UIMode") == "Full") 
			{ // only when kidgames are supported by system+theme, and when in full UImode
				if (cursor->getType() == GAME) {
					mKidGameChange = true;
					MetaDataList* md = &cursor->metadata;
					std::string value = md->get("kidgame");
					LOG(LogDebug) << "kidgame = "<< value;
					if (value.compare("false") == 0) {
						md->set("kidgame", "true");
					}					else {
						md->set("kidgame", "false");
					}
					LOG(LogDebug) << "New kidgame value set to: "<< md->get("kidgame");
					updateInfoPanel();
				}
			}
		}else if(config->isMappedTo("right", input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				if (mFavoriteChange || mKidGameChange)
				{
					ViewController::get()->setInvalidGamesList(getCursor()->getSystem());
					mFavoriteChange = false;
					mKidGameChange = false;
				}
				ViewController::get()->goToNextGameList();
				return true;
			}
		}else if(config->isMappedTo("left", input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				if (mFavoriteChange || mKidGameChange)
				{
					ViewController::get()->setInvalidGamesList(getCursor()->getSystem());
					mFavoriteChange = false;
					mKidGameChange = false;
				}
				ViewController::get()->goToPrevGameList();
				return true;
			}
		}
	}

	return IGameListView::input(config, input);
}
