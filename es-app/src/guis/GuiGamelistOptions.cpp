#include "GuiGamelistOptions.h"
#include "GuiMetaDataEd.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"

GuiGamelistOptions::GuiGamelistOptions(Window* window, SystemData* system) : GuiComponent(window),
	mSystem(system), mMenu(window, "OPTIONS"), fromPlaceholder(false), mFiltersChanged(false)
{
	addChild(&mMenu);

	// check it's not a placeholder folder - if it is, only show "Filter Options"
	FileData* file = getGamelist()->getCursor();
	fromPlaceholder = file->isPlaceHolder();
	bool isFiltered = system->getIndex()->isFiltered();
	ComponentListRow row;

	// show filtered menu
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "FILTER GAMELIST", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.addElement(makeArrow(mWindow), false);
	row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openGamelistFilter, this));
	mMenu.addRow(row);

	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "SURPRISE ME!", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [&](InputConfig* config, Input input) {
		if (config->isMappedTo("a", input) && input.value)
		{
			ViewController::get()->goToRandomGame();
			delete this;
			return true;
		}
		return false;
	};
	mMenu.addRow(row);

	if (!fromPlaceholder) {

		if (!isFiltered) {
			// jump to letter
			row.elements.clear();
			char curChar = toupper(getGamelist()->getCursor()->getName()[0]);
			if(curChar < 'A' || curChar > 'Z')
				curChar = 'A';

			mJumpToLetterList = std::make_shared<LetterList>(mWindow, "JUMP TO LETTER", false);
			for(char c = 'A'; c <= 'Z'; c++)
				mJumpToLetterList->add(std::string(1, c), c, c == curChar);

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
		}

		// sort list by
		mListSort = std::make_shared<SortList>(mWindow, "SORT GAMES BY", false);
		for(unsigned int i = 0; i < FileSorts::SortTypes.size(); i++)
		{
			const FileData::SortType& sort = FileSorts::SortTypes.at(i);
			mListSort->add(sort.description, &sort, i == 0); // TODO - actually make the sort type persistent
		}

		mMenu.addWithLabel("SORT GAMES BY", mListSort);

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
	// apply sort
	if (!fromPlaceholder) {
		FileData* root = getGamelist()->getCursor()->getSystem()->getRootFolder();
		root->sort(*mListSort->getSelected()); // will also recursively sort children

		// notify that the root folder was sorted
		getGamelist()->onFileChanged(root, FILE_SORTED);
	}
	if (mFiltersChanged)
	{
		if (!fromPlaceholder) {
			FileData* root = getGamelist()->getCursor()->getSystem()->getRootFolder();
			getGamelist()->onFileChanged(root, FILE_SORTED);
		}
		else
		{
			// only reload full view if we came from a placeholder
			// as we need to re-display the remaining elements for whatever new
			// game is selected
			ViewController::get()->reloadGameListView(mSystem);
		}
	}
}

void GuiGamelistOptions::openGamelistFilter()
{
	mFiltersChanged = true;
	GuiGamelistFilter* ggf = new GuiGamelistFilter(mWindow, mSystem);
	mWindow->pushGui(ggf);
}

void GuiGamelistOptions::openMetaDataEd()
{
	// open metadata editor
	FileData* file = getGamelist()->getCursor();
	ScraperSearchParams p;
	p.game = file;
	p.system = file->getSystem();

	std::function<void()> deleteBtnFunc;

	if (file->getType() == FOLDER)
	{
		deleteBtnFunc = NULL;
	}
	else
	{
		deleteBtnFunc = [this, file] {
			getGamelist()->remove(file);
		};
	}

	mWindow->pushGui(new GuiMetaDataEd(mWindow, &file->metadata, file->metadata.getMDD(), p, file->getPath().filename().string(),
		std::bind(&IGameListView::onFileChanged, getGamelist(), file, FILE_METADATA_CHANGED), deleteBtnFunc));
}

void GuiGamelistOptions::jumpToLetter()
{
	char letter = mJumpToLetterList->getSelected();
	IGameListView* gamelist = getGamelist();

	// this is a really shitty way to get a list of files
	const std::vector<FileData*>& files = gamelist->getCursor()->getParent()->getChildren();

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
		delete this;
		return true;
	}

	return mMenu.input(config, input);
}

HelpStyle GuiGamelistOptions::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(mSystem->getTheme(), "system");
	return style;
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
