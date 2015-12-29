#include "GuiGamelistOptions.h"
#include "GuiMetaDataEd.h"
#include "GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "guis/GuiTextEditPopup.h"
#include "VolumeControl.h"
#include "SystemManager.h"
#include "Settings.h"

GuiGamelistOptions::GuiGamelistOptions(Window* window, SystemData* system) : GuiComponent(window), 
	mSystem(system), 
	mMenu(window, "MENU")
{
	addChild(&mMenu);
	ComponentListRow row;
	// volume
	mVolume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
	mVolume->setValue((float)VolumeControl::getInstance()->getVolume());
	mMenu.addWithLabel("SYSTEM VOLUME", mVolume);
	// sort list by
	mListSort = std::make_shared<SortList>(mWindow, "SORT GAMES BY", false);

	const std::vector<FileSort>& sorts = getFileSorts();
	for(unsigned int i = 0; i < sorts.size(); i++)
	{
		const FileSort& sort = sorts.at(i);
		mListSort->add(sort.description, i, i == Settings::getInstance()->getInt("SortTypeIndex"));
	}

	mMenu.addWithLabel("SORT GAMES BY", mListSort);
	if (system)
	{

/*
		// jump to letter
		char curChar = toupper(getGamelist()->getCursor().getName()[0]);
		if(curChar < 'A' || curChar > 'Z') // in the case of unicode characters, pretend it's an A
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
*/



		// edit game metadata
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, "EDIT THIS GAME'S METADATA", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.addElement(makeArrow(mWindow), false);
		row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openMetaDataEd, this));
		mMenu.addRow(row); 
		// add a filter entry to our current list
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, "ADD FILTER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.addElement(makeArrow(mWindow), false);
		row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openFilterAddStart, this));
		mMenu.addRow(row);
	} //End Gamelist Specific entries

	addEntry("SETTINGS", 0x777777FF, true,
		[this] {
		auto s = new GuiMenu(mWindow);
		mWindow->pushGui(s);
	});
	addEntry("QUIT", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "QUIT");
			
			Window* window = mWindow;

			ComponentListRow row;
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES", 
				[] { 
					if(runRestartCommand() != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES", 
				[] { 
					if(runShutdownCommand() != 0)
						LOG(LogWarning) << "Shutdown terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			if(Settings::getInstance()->getBool("ShowExit"))
			{
				row.elements.clear();
				row.makeAcceptInputHandler([window] {
					window->pushGui(new GuiMsgBox(window, "REALLY QUIT?", "YES", 
					[] { 
						SDL_Event ev;
						ev.type = SDL_QUIT;
						SDL_PushEvent(&ev);
					}, "NO", nullptr));
				});
				row.addElement(std::make_shared<TextComponent>(window, "QUIT EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
				s->addRow(row);
			}

			mWindow->pushGui(s);
	});
	mMenu.addButton("BACK", "go back", [this] { delete this; });
	// center the menu
	setSize(mMenu.getSize());
	mMenu.setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

GuiGamelistOptions::~GuiGamelistOptions()
{
	// apply sort if it changed
	if(mSystem && mListSort->getSelected() != Settings::getInstance()->getInt("SortTypeIndex"))
	{
		Settings::getInstance()->setInt("SortTypeIndex", mListSort->getSelected());
		ViewController::get()->onFilesChanged(NULL);
	}
	VolumeControl::getInstance()->setVolume((int)round(mVolume->getValue()));
}


void GuiGamelistOptions::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

void GuiGamelistOptions::openMetaDataEd()
{
	// open metadata editor
	const FileData& file = getGamelist()->getCursor();
	ScraperSearchParams p(file.getSystem(), file);
	auto deleteFunc = [this, file] {
		boost::filesystem::remove(file.getPath()); // actually delete the file on the filesystem
		SystemManager::getInstance()->database().removeEntry(file); // update the database
		getGamelist()->onFilesChanged(); // tell the view
	};
        auto removeFunc = [this, file] {
		SystemManager::getInstance()->database().removeEntry(file); // update the database
		getGamelist()->onFilesChanged(); // tell the view
	};
	mWindow->pushGui(new GuiMetaDataEd(mWindow, file, 
		std::bind(&IGameListView::onMetaDataChanged, getGamelist(), file), deleteFunc, removeFunc));
}

void GuiGamelistOptions::openFilterAddStart()
{
	// open a dialog to add a filter
	const FileData& file = getGamelist()->getParentCursor();
	auto updateVal = [this, file](const std::string& filterid) { 
          this->openFilterAdd(file,filterid);
        };
	mWindow->pushGui(new GuiTextEditPopup(mWindow, "ENTER FILTER'S FILE NAME", "", updateVal, false));
	

	
}

void GuiGamelistOptions::openFilterAdd(const FileData& fileParent, const std::string &filterid)
{
        if(filterid.empty()) return;
        FileData filter(fileParent.getFileID() + "/" + filterid, fileParent.getSystemID(), FILTER);
        MetaDataMap emptyfiltermetadata(FILTER_METADATA);
        filter.set_metadata(emptyfiltermetadata);
	auto deleteFunc = [this, filter] {
		SystemManager::getInstance()->database().removeEntry(filter); // update the database
		getGamelist()->onFilesChanged(); // tell the view
	};
	mWindow->pushGui(new GuiMetaDataEd(mWindow, filter, 
		std::bind(&IGameListView::onMetaDataChanged, getGamelist(), filter), deleteFunc, nullptr));
}
void GuiGamelistOptions::jumpToLetter()
{
	char letter = mJumpToLetterList->getSelected();
	IGameListView* gamelist = getGamelist();

	// this is a really shitty way to get a list of files
	// TODO
	/*
	const std::vector<FileData>& files = gamelist->getCursor()->getParent()->getChildren();
	
	long min = 0;
	long max = files.size() - 1;
	long mid = 0;

	while(max >= min)
	{
		mid = ((max - min) / 2) + min;

		// game somehow has no first character to check
		const std::string& name = files.at(mid).getName();
		if(name.empty())
			continue;

		char checkLetter = toupper(name[0]);

		if(checkLetter < letter)
			min = mid + 1;
		else if(checkLetter > letter)
			max = mid - 1;
		else
			break; //exact match found
	}

	gamelist->setCursor(files.at(mid));
	*/

	delete this;
}

bool GuiGamelistOptions::input(InputConfig* config, Input input)
{
	if((config->isMappedTo("b", input)) && input.value)
	{
		delete this;
		return true;
	}
	
	if((config->isMappedTo("select", input) || config->isMappedTo("start",input)) && input.value != 0)
	{
		// close everything
		Window* window = mWindow;
		while(window->peekGui() && window->peekGui() != ViewController::get())
			delete window->peekGui();
		return true;
	}
	

	return mMenu.input(config, input);
}

std::vector<HelpPrompt> GuiGamelistOptions::getHelpPrompts()
{
	auto prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("select", "close"));
	return prompts;
}

IGameListView* GuiGamelistOptions::getGamelist()
{
	return ViewController::get()->getGameListView(mSystem).get();
}
