#include "guis/GuiMetaDataEd.h"

#include <stdlib.h>
#include "components/ButtonComponent.h"
#include "components/ComponentList.h"
#include "components/DateTimeComponent.h"
#include "components/DateTimeEditComponent.h"
#include "components/MenuComponent.h"
#include "components/RatingComponent.h"
#include "components/SwitchComponent.h"
#include "components/TextComponent.h"
#include "guis/GuiGameScraper.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiTextEditPopup.h"
#include "resources/Font.h"
#include "utils/StringUtil.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "FileData.h"
#include "FileFilterIndex.h"
#include "SystemData.h"
#include "Window.h"
#include "Log.h"

GuiMetaDataEd::GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams scraperParams,
	const std::string& /*header*/, std::function<void()> saveCallback, std::function<void()> deleteFunc) : GuiComponent(window),
	mScraperParams(scraperParams),

	mBackground(window, ":/frame.png"),
	mGrid(window, Vector2i(1, 3)),

	mMetaDataDecl(mdd),
	mMetaData(md),
	mSavedCallback(saveCallback),
	mDeleteFunc(deleteFunc)
{
	addChild(&mBackground);
	addChild(&mGrid);

	mHeaderGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i(1, 5));

	mTitle = std::make_shared<TextComponent>(mWindow, "EDIT METADATA", Font::get(FONT_SIZE_LARGE), 0x555555FF, ALIGN_CENTER);
	std::string tgt = md->getType() == GAME_METADATA ? "GAME" : "FOLDER";
	std::string subt = tgt + ": " + Utils::String::toUpper(Utils::FileSystem::getFileName(scraperParams.game->getPath()));
	mSubtitle = std::make_shared<TextComponent>(mWindow, subt, Font::get(FONT_SIZE_SMALL), 0x777777FF, ALIGN_CENTER);
	mHeaderGrid->setEntry(mTitle, Vector2i(0, 1), false, true);
	mHeaderGrid->setEntry(mSubtitle, Vector2i(0, 3), false, true);

	mGrid.setEntry(mHeaderGrid, Vector2i(0, 0), false, true);

	mList = std::make_shared<ComponentList>(mWindow);
	mGrid.setEntry(mList, Vector2i(0, 1), true, true);

	// populate list
	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
	{
		// don't add statistics
		if(iter->isStatistic)
			continue;

		std::shared_ptr<GuiComponent> ed;

		// create ed and add it (and any related components) to mMenu
		// ed's value will be set below
		ComponentListRow row;
		auto lblTxt = Utils::String::toUpper(iter->displayName);
		if (iter->type == MD_DATE) {
			lblTxt += " (" + DateTimeComponent::getDateformatTip() + ")";
		}
		auto lbl = std::make_shared<TextComponent>(mWindow, lblTxt, Font::get(FONT_SIZE_SMALL), 0x777777FF);
		row.addElement(lbl, true); // label

		switch(iter->type)
		{
		case MD_BOOL:
			{
				ed = std::make_shared<SwitchComponent>(window);
				row.addElement(ed, false, true);
				break;
			}
		case MD_RATING:
			{
				ed = std::make_shared<RatingComponent>(window);
				const float height = lbl->getSize().y() * 0.71f;
				ed->setSize(0, height);
				row.addElement(ed, false, true);

				auto spacer = std::make_shared<GuiComponent>(mWindow);
				spacer->setSize(Renderer::getScreenWidth() * 0.0025f, 0);
				row.addElement(spacer, false);

				// pass input to the actual RatingComponent instead of the spacer
				row.input_handler = std::bind(&GuiComponent::input, ed.get(), std::placeholders::_1, std::placeholders::_2);

				break;
			}
		case MD_DATE:
			{
				ed = std::make_shared<DateTimeEditComponent>(window);
				row.addElement(ed, false);

				auto spacer = std::make_shared<GuiComponent>(mWindow);
				spacer->setSize(Renderer::getScreenWidth() * 0.0025f, 0);
				row.addElement(spacer, false);

				// pass input to the actual DateTimeEditComponent instead of the spacer
				row.input_handler = std::bind(&GuiComponent::input, ed.get(), std::placeholders::_1, std::placeholders::_2);

				break;
			}
		case MD_TIME:
			{
				ed = std::make_shared<DateTimeEditComponent>(window, DateTimeEditComponent::DISP_RELATIVE_TO_NOW);
				row.addElement(ed, false);
				break;
			}
		case MD_MULTILINE_STRING:
		default:
			{
				// MD_STRING
				ed = std::make_shared<TextComponent>(window, "", Font::get(FONT_SIZE_SMALL, FONT_PATH_LIGHT), 0x777777FF, ALIGN_RIGHT);
				const float height = lbl->getSize().y() * 0.71f;
				ed->setSize(0, height);
				row.addElement(ed, true);

				auto spacer = std::make_shared<GuiComponent>(mWindow);
				spacer->setSize(Renderer::getScreenWidth() * 0.005f, 0);
				row.addElement(spacer, false);

				auto bracket = std::make_shared<ImageComponent>(mWindow);
				bracket->setImage(":/arrow.svg");
				bracket->setResize(Vector2f(0, lbl->getFont()->getLetterHeight()));
				row.addElement(bracket, false);

				bool multiLine = iter->type == MD_MULTILINE_STRING;
				const std::string title = iter->displayPrompt;
				auto updateVal = [ed](const std::string& newVal) { ed->setValue(newVal); }; // ok callback (apply new value to ed)
				row.makeAcceptInputHandler([this, title, ed, updateVal, multiLine] {
					mWindow->pushGui(new GuiTextEditPopup(mWindow, title, ed->getValue(), updateVal, multiLine));
				});
				break;
			}
		}

		assert(ed);
		mList->addRow(row);
		ed->setValue(mMetaData->get(iter->key));
		mEditors.push_back(ed);
	}

	std::vector< std::shared_ptr<ButtonComponent> > buttons;

	if(md->getType() == GAME_METADATA && !scraperParams.system->hasPlatformId(PlatformIds::PLATFORM_IGNORE))
		buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "SCRAPE", "scrape", std::bind(&GuiMetaDataEd::fetch, this)));

	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "SAVE", "save", [&] { save(); delete this; }));
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "CANCEL", "cancel", [&] { delete this; }));

	if(mDeleteFunc)
	{
		auto deleteFileAndSelf = [&] { mDeleteFunc(); delete this; };
		auto deleteBtnFunc = [this, deleteFileAndSelf] { mWindow->pushGui(new GuiMsgBox(mWindow, "THIS WILL DELETE THE ACTUAL GAME FILE(S)!\nARE YOU SURE?", "YES", deleteFileAndSelf, "NO", nullptr)); };
		buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "DELETE", "delete", deleteBtnFunc));
	}

	mButtons = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtons, Vector2i(0, 2), true, false);

	// resize + center
	float width = (float)Math::min(Renderer::getScreenHeight(), (int)(Renderer::getScreenWidth() * 0.90f));
	setSize(width, Renderer::getScreenHeight() * 0.82f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiMetaDataEd::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	mGrid.setSize(mSize);

	const float titleHeight = mTitle->getFont()->getLetterHeight();
	const float subtitleHeight = mSubtitle->getFont()->getLetterHeight();
	const float titleSubtitleSpacing = mSize.y() * 0.03f;

	mGrid.setRowHeightPerc(0, (titleHeight + titleSubtitleSpacing + subtitleHeight + TITLE_VERT_PADDING) / mSize.y());
	mGrid.setRowHeightPerc(2, mButtons->getSize().y() / mSize.y());

	mHeaderGrid->setRowHeightPerc(1, titleHeight / mHeaderGrid->getSize().y());
	mHeaderGrid->setRowHeightPerc(2, titleSubtitleSpacing / mHeaderGrid->getSize().y());
	mHeaderGrid->setRowHeightPerc(3, subtitleHeight / mHeaderGrid->getSize().y());
}

void GuiMetaDataEd::save()
{
	// remove game from index
	mScraperParams.system->getIndex()->removeFromIndex(mScraperParams.game);

	assert(mMetaDataDecl.size() >= mEditors.size());
	// there may be less editfields than metadata entries as
	// statistic md fields are not shown to the user.
	// md statistic fields are not necessarily at the end of the md list
	int edIdx = 0;
	for(auto &mdd : mMetaDataDecl)
	{
		if(!mdd.isStatistic) {
			mMetaData->set(mdd.key, mEditors.at(edIdx)->getValue());
			edIdx++;
		}
	}

	// enter game in index
	mScraperParams.system->getIndex()->addToIndex(mScraperParams.game);

	if(mSavedCallback)
		mSavedCallback();

	// update respective Collection Entries
	CollectionSystemManager::get()->refreshCollectionSystems(mScraperParams.game);

	mScraperParams.system->onMetaDataSavePoint();
}

void GuiMetaDataEd::fetch()
{
	GuiGameScraper* scr = new GuiGameScraper(mWindow, mScraperParams, std::bind(&GuiMetaDataEd::fetchDone, this, std::placeholders::_1));
	mWindow->pushGui(scr);
}

void GuiMetaDataEd::fetchDone(const ScraperSearchResult& result)
{
	assert(mMetaDataDecl.size() >= mEditors.size());
	int edIdx = 0;
	for(auto &mdd : mMetaDataDecl)
	{
		if(mdd.isStatistic)
			continue;

		mEditors.at(edIdx)->setValue(result.mdl.get(mdd.key));
		edIdx++;
	}
}

void GuiMetaDataEd::close(bool closeAllWindows)
{
	bool dirty = hasChanges();

	std::function<void()> closeFunc;
	if(!closeAllWindows)
	{
		closeFunc = [this] { delete this; };
	}else{
		Window* window = mWindow;
		closeFunc = [window, this] {
			while(window->peekGui() != ViewController::get())
				delete window->peekGui();
		};
	}

	if(dirty)
	{
		// changes were made, ask if the user wants to save them
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"SAVE CHANGES?",
			"YES", [this, closeFunc] { save(); closeFunc(); },
			"NO", closeFunc
		));
	}else{
		closeFunc();
	}
}

bool GuiMetaDataEd::hasChanges()
{
	assert(mMetaDataDecl.size() >= mEditors.size());
	// find out if the user made any changes
	int edIdx = 0;
	for(auto &mdd : mMetaDataDecl)
	{
		if(!mdd.isStatistic)
		{
			std::string gamelistVal = mMetaData->get(mdd.key);
			std::string editorVal = mEditors.at(edIdx++)->getValue();
			if (mdd.key == "rating")
			{
				// needed to catch "0", "0.0" or ".<d>" (and "1.0") from gamelist string rating
				// getValue() of RatingComponent returns "0" for floats 0, 0.0; "0.<d>" for .<d>
				// and "1" for float 1.0
				// convert to float and compare to avoid false "Save Changes" prompt
				bool ok;
				if (to_float(gamelistVal, ok) != to_float(editorVal, ok))
					return true;
			}
			else
			{
				// string compare
				if (gamelistVal != editorVal)
					return true;
			}
		}
	}
	return false;
}

float GuiMetaDataEd::to_float(const std::string& str, bool& ok)
{
	errno = 0;
	char* end = nullptr;
	float f = std::strtof(str.c_str(), &end);
	ok = !str.empty() && !*end && errno == 0;
	if (!ok)
		LOG(LogWarning) << "Conversion of input string '" << str << "' to float failed or is incomplete. Return value: " << f;
	return f;
}

bool GuiMetaDataEd::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	const bool isStart = config->isMappedTo("start", input);
	if(input.value != 0 && (config->isMappedTo("b", input) || isStart))
	{
		close(isStart);
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiMetaDataEd::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mGrid.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
