#include "views/gamelist/ISimpleGameListView.h"
#include "ThemeData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Sound.h"
#include "Settings.h"
#include "ArchiveManager.h"
#include <algorithm>
#include "PlatformId.h"
#include "SystemData.h"
#include "guis/GuiMsgBox.h"

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) : IGameListView(window, root),
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

			if(ArchiveManager::isAnArchive(cursor->getPath().native()) &&
			   !(cursor->getSystem()->hasPlatformId(PlatformIds::ARCADE) ||
				 cursor->getSystem()->hasPlatformId(PlatformIds::NEOGEO)))
			{
				// Extraction in /tmp/somefolder
				ArchiveManager mgr{cursor->getPath().native()};
				decltype(mgr.list()) filelist;
				std::string extractedFolder;

				try
				{
					extractedFolder = mgr.extract();
					filelist = mgr.list();
				}
				catch(std::runtime_error& e)
				{
					std::cerr << e.what() << std::endl;
					mWindow->pushGui(new GuiMsgBox(mWindow, "An error occured while opening the archive.", "OK"));
					return true;
				}

				// Add to the list so that we can enter it
				auto f = new FileData(FOLDER, extractedFolder, cursor->getSystem());
				f->setTemporary();

				cursor->getParent()->addChild(f);

				// Add the files inside the archive
				for(auto& e : filelist)
				{
					f->addChild(new FileData(GAME, extractedFolder + "/" + e, cursor->getSystem()));
				}

				mCursorStack.push(f);
				populateList(f->getChildren());
			}
			else if(cursor->getType() == GAME)
			{
				Sound::getFromTheme(getTheme(), getName(), "launch")->play();
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
				auto parent = mCursorStack.top()->getParent();
				if(mCursorStack.top()->isTemporary())
				{
					mCursorStack.top()->getParent()->removeChild(mCursorStack.top());
					populateList(parent->getChildren());
					setCursor(parent->getChildren()[0]); // TODO save the name of the archive somewhere
				}
				else
				{
					populateList(parent->getChildren());
					setCursor(mCursorStack.top());
				}

				mCursorStack.pop();

				Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}else{
				onFocusLost();
				ViewController::get()->goToSystemView(getCursor()->getSystem());
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
