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
	LOG(LogDebug) << "GUIGamelistOptions::GuiGamelistOptions()";
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

	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "SURPRISE ME!", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [&](InputConfig* config, Input input) {
		if(config->isMappedTo("a", input) && input.value)
		{
			SurpriseMe();
			return true;
		}
		return false;
	};
	mMenu.addRow(row);

	// sort list by
	mListSort = std::make_shared<SortList>(mWindow, "SORT GAMES BY", false);
	for(unsigned int i = 0; i < FileSorts::SortTypes.size(); i++)
	{
		const FileData::SortType& sort = FileSorts::SortTypes.at(i);
		mListSort->add(sort.description, &sort, i == 0); // TODO - actually make the sort type persistent
	}

	mMenu.addWithLabel("SORT GAMES BY", mListSort);

	// Toggle: Show favorites-only
	auto favorite_only = std::make_shared<SwitchComponent>(mWindow);
	favorite_only->setState(Settings::getInstance()->getBool("FavoritesOnly"));
	mMenu.addWithLabel("FAVORITES ONLY", favorite_only);
	addSaveFunc([favorite_only, this] {

		// First save to settings, in order for filtering to work
		if (favorite_only->getState() != Settings::getInstance()->getBool("FavoritesOnly"))
		{
			Settings::getInstance()->setBool("FavoritesOnly", favorite_only->getState());
			mFavoriteStateChanged = true;
		}
		if(favorite_only->getState())
		{
			bool hasFavorite = false;
			// check if there is anything at all to show, otherwise revert
			for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); it++) {
				if( (*it)->getGameCount(true) > 0 ) {
					hasFavorite = true;
					break;
				}
			}
			if(!hasFavorite)
			{
				LOG(LogDebug) << "Nothing to show in selected favorites mode, resetting";
				favorite_only->setState(false);
				Settings::getInstance()->setBool("FavoritesOnly", favorite_only->getState());
			}
		}
	});

	// Toggle: Show hidden
	if(Settings::getInstance()->getString("UIMode") == "Full")
	{
		auto show_hidden = std::make_shared<SwitchComponent>(mWindow);
		show_hidden->setState(Settings::getInstance()->getBool("ShowHidden"));
		mMenu.addWithLabel("SHOW HIDDEN", show_hidden);
		addSaveFunc([show_hidden, this] {
			// First save to settings, in order for filtering to work
			if(show_hidden->getState() != Settings::getInstance()->getBool("ShowHidden"))
			{
				Settings::getInstance()->setBool("ShowHidden", show_hidden->getState());
				mHiddenStateChanged = true;
			}
			if(!show_hidden->getState())
			{
				bool hasNonHidden = false;
				for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); it++) {
					if( (*it)->getGameCount(true) > 0 ) {
						hasNonHidden = true;
						break;
					}
				}
				if(!hasNonHidden)
				{
					LOG(LogDebug) << "Nothing to show in selected show hidden mode, resetting";
					show_hidden->setState(true);
					Settings::getInstance()->setBool("ShowHidden", show_hidden->getState());
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
	// apply sort
	FileData* root = getGamelist()->getCursor()->getSystem()->getRootFolder();
	root->sort(*mListSort->getSelected()); // will also recursively sort children

	// notify that the root folder was sorted

	getGamelist()->onFileChanged(root, FILE_SORTED);
	if (mFavoriteStateChanged || mHiddenStateChanged)
	{
		if (mFavoriteStateChanged) {
			LOG(LogDebug) << "  GUIGamelistOptions::~GuiGamelistOptions(): FavoriteStateChanged, reloading GameList";
		}
		if (mHiddenStateChanged) {
			LOG(LogDebug) << "  GUIGamelistOptions::~GuiGamelistOptions(): HiddenStateChanged, reloading GameList";
		}
		ViewController::get()->setAllInvalidGamesList(getGamelist()->getCursor()->getSystem());
		//ViewController::get()->reloadGameListView(getGamelist()->getCursor()->getSystem());
		ViewController::get()->reloadAll();
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

void GuiGamelistOptions::SurpriseMe()
{
	LOG(LogDebug) << "GuiGamelistOptions::SurpriseMe()";
	ViewController::get()->goToRandomGame();
	delete this;
}
