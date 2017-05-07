#include "GuiGamelistOptions.h"
#include "GuiMetaDataEd.h"
#include "Settings.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "components/SwitchComponent.h"
#include "guis/GuiSettings.h"
#include "Log.h"
#include "SystemData.h"
#include "FileData.h"

GuiGamelistOptions::GuiGamelistOptions(Window* window, SystemData* system) : GuiComponent(window),
	mSystem(system),
	mMenu(window, "OPTIONS")
{
	addChild(&mMenu);

	// jump to letter
	char curChar = toupper(getGamelist()->getCursor()->getName()[0]);
	if(curChar < 'A' || curChar > 'Z')
		curChar = 'A';

	mJumpToLetterList = std::make_shared<LetterList>(mWindow, "JUMP TO LETTER", false);
	for(char c = 'A'; c <= 'Z'; c++)
		mJumpToLetterList->add(std::string(1, c), c, c == curChar);

	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, "JUMP TO LETTER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.addElement(mJumpToLetterList, false);
	row.input_handler = [&](InputConfig* config, Input input) {
		if(config->isMappedTo("a", input) && input.value)
		{
			jumpToLetter();
			return true;
		}
		else if(mJumpToLetterList->input(config, input))
		{
			return true;
		}
		return false;
	};
	mMenu.addRow(row);

	// sort list by
	std::shared_ptr<SortList> list_sorts = std::make_shared<SortList>(mWindow, "SORT GAMES BY", false);
	for(unsigned int i = 0; i < FileSorts::SortTypes.size(); i++)
	{
		const FileData::SortType& sort = FileSorts::SortTypes.at(i);
		list_sorts->add(sort.description, &sort, Settings::getInstance()->getString("SortMode") == sort.description);
	}

	mMenu.addWithLabel("SORT GAMES BY", list_sorts);
	addSaveFunc([list_sorts, this]{
		if (list_sorts->getSelected()->description != Settings::getInstance()->getString("SortMode"))
		{
			Settings::getInstance()->setString("SortMode", list_sorts->getSelected()->description);
			FileData* root = getGamelist()->getCursor()->getSystem()->getRootFolder();
			root->sort(*list_sorts->getSelected());
			getGamelist()->onFileChanged(root, FILE_SORTED); // notify that the root folder was sorted
		}
	});
	
	// Toggle: Show favorites-only
	auto favorite_only = std::make_shared<SwitchComponent>(mWindow);
	favorite_only->setState(Settings::getInstance()->getBool("FavoritesOnly"));
	mMenu.addWithLabel("FAVORITES ONLY", favorite_only);
	addSaveFunc([favorite_only, this] {
		if (favorite_only->getState() != Settings::getInstance()->getBool("FavoritesOnly"))
		{
			Settings::getInstance()->setBool("FavoritesOnly", favorite_only->getState());	// First save to settings, in order for filtering to work
			if(! mSystem->isValidFilter())													// then check if there is anything at all to show in any system
			{
				LOG(LogDebug) << "Nothing to show in selected favorites mode, resetting";	// if not, then revert chenge.
				Settings::getInstance()->setBool("FavoritesOnly", false);
			} else
			{
				mViewStateChanged = true;
			}
		}
	});

	// Toggle: Show hidden, only in Full UI mode
	if(Settings::getInstance()->getString("UIMode") == "Full")
	{
		auto show_hidden = std::make_shared<SwitchComponent>(mWindow);
		show_hidden->setState(Settings::getInstance()->getBool("ShowHidden"));
		mMenu.addWithLabel("SHOW HIDDEN", show_hidden);
		addSaveFunc([show_hidden, this] {
			
			if(show_hidden->getState() != Settings::getInstance()->getBool("ShowHidden"))
			{
				Settings::getInstance()->setBool("ShowHidden", show_hidden->getState());			// First save to settings, in order for filtering to work
				if (!mSystem->isValidFilter())														// then check if there is anything at all to show in any system
				{
					LOG(LogDebug) << "Nothing to show when not showing hidden files, resetting";	// if not, then revert change.
					Settings::getInstance()->setBool("ShowHidden", true);
				} else
				{
					mViewStateChanged = true;
				}
			}
		});
	}

	// edit game metadata - only in Full UI mode
	if(Settings::getInstance()->getString("UIMode") == "Full")
	{
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, "EDIT THIS GAME'S METADATA", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.addElement(makeArrow(mWindow), false);
		row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openMetaDataEd, this));
		mMenu.addRow(row);
	}

	// center the menu
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, (mSize.y() - mMenu.getSize().y()) / 2);
}

GuiGamelistOptions::~GuiGamelistOptions()
{
	LOG(LogDebug) << "GUIGamelistOptions::~GuiGamelistOptions()";
	if (mViewStateChanged)
		refreshView();

}

void GuiGamelistOptions::refreshView()
{
	LOG(LogDebug) << " Updating filter mode, or edited metadata, notifying ViewController";
	FileData* root = mSystem->getRootFolder();
	ViewController::get()->onFileChanged(root, FILE_FILTERED);

	if (mSystem->getGameCount(true) == 0) {											//still, the current gamelist might be empty, so lets check and otherwise revert
		LOG(LogDebug) << " Whoops, nothing to see here, returning to start.";
		ViewController::get()->goToStart();
	}
}

void GuiGamelistOptions::openMetaDataEd()
{
	// open metadata editor
	FileData* file = getGamelist()->getCursor();
	ScraperSearchParams p;
	p.game = file;
	p.system = file->getSystem();
	mWindow->pushGui(new GuiMetaDataEd(mWindow, &file->metadata, file->metadata.getMDD(), p, file->getPath().filename().string(),
		std::bind(&IGameListView::onFileChanged, getGamelist(), file, FILE_METADATA_CHANGED), [this, file] {
			getGamelist()->remove(file);
	}));
	
	//This might be too costly, but is a failsave way to trigger a game-list refresh.
	mViewStateChanged = true;
}

void GuiGamelistOptions::jumpToLetter()
{
	char letter = mJumpToLetterList->getSelected();
	IGameListView* gamelist = getGamelist();

	// this is a really shitty way to get a list of files
	const std::vector<FileData*>& files = gamelist->getCursor()->getParent()->getChildren(true);

	long min = 0;
	long max = files.size() - 1;
	long mid = 0;

	while(max >= min)
	{
		mid = ((max - min) / 2) + min;

		// game somehow has no first character to check
		if(files.at(mid)->getName().empty())
			continue;

		char checkLetter = toupper(files.at(mid)->getName()[0]);

		if(checkLetter < letter)
			min = mid + 1;
		else if(checkLetter > letter || (mid > 0 && (letter == toupper(files.at(mid - 1)->getName()[0]))))
			max = mid - 1;
		else
			break; //exact match found
	}

	gamelist->setCursor(files.at(mid));

	delete this;
}

bool GuiGamelistOptions::input(InputConfig* config, Input input)
{
	if((config->isMappedTo("b", input) || config->isMappedTo("select", input)) && input.value)
	{
		save();
		delete this;
		return true;
	}

	return mMenu.input(config, input);
}

std::vector<HelpPrompt> GuiGamelistOptions::getHelpPrompts()
{
	auto prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "close"));
	return prompts;
}

IGameListView* GuiGamelistOptions::getGamelist()
{
	return ViewController::get()->getGameListView(mSystem).get();
}

void GuiGamelistOptions::save()
{
	if (!mSaveFuncs.size())
		return;

	for (auto it = mSaveFuncs.begin(); it != mSaveFuncs.end(); it++)
		(*it)();

	Settings::getInstance()->saveFile();
}
