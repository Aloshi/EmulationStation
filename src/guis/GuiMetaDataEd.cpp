#include "GuiMetaDataEd.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../components/AsyncReqComponent.h"
#include "../Settings.h"
#include "GuiGameScraper.h"
#include "GuiMsgBox.h"
#include <boost/filesystem.hpp>

#include "../components/TextEditComponent.h"
#include "../components/DateTimeComponent.h"
#include "../components/RatingComponent.h"
#include "GuiTextEditPopup.h"

using namespace Eigen;

GuiMetaDataEd::GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams scraperParams, 
	const std::string& header, std::function<void()> saveCallback, std::function<void()> deleteFunc) : GuiComponent(window), 
	mScraperParams(scraperParams), 

	mBackground(window, ":/frame.png"), 
	mGrid(window, Vector2i(1, 3)),

	mMetaDataDecl(mdd), 
	mMetaData(md), 
	mSavedCallback(saveCallback), mDeleteFunc(deleteFunc)
{
	addChild(&mBackground);
	addChild(&mGrid);

	mHeaderGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i(1, 5));
	
	mTitle = std::make_shared<TextComponent>(mWindow, "EDIT METADATA", Font::get(FONT_SIZE_LARGE), 0x333333FF, TextComponent::ALIGN_CENTER);
	mSubtitle = std::make_shared<TextComponent>(mWindow, strToUpper(scraperParams.game->getPath().filename().generic_string()), 
		Font::get(FONT_SIZE_SMALL), 0x777777FF, TextComponent::ALIGN_CENTER);
	mHeaderGrid->setEntry(mTitle, Vector2i(0, 1), false, true);
	mHeaderGrid->setEntry(mSubtitle, Vector2i(0, 3), false, true);

	mGrid.setEntry(mHeaderGrid, Vector2i(0, 0), false, true);

	mList = std::make_shared<ComponentList>(mWindow);
	mGrid.setEntry(mList, Vector2i(0, 1), true, true);

	// populate list
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		std::shared_ptr<GuiComponent> ed;

		// don't add statistics
		if(iter->isStatistic)
			continue;

		// create ed and add it (and any related components) to mMenu
		// ed's value will be set below
		ComponentListRow row;
		auto lbl = std::make_shared<TextComponent>(mWindow, strToUpper(iter->displayName), Font::get(FONT_SIZE_SMALL), 0x777777FF);
		row.addElement(lbl, true); // label

		switch(iter->type)
		{
		case MD_RATING:
			{
				ed = std::make_shared<RatingComponent>(window);
				const float height = lbl->getSize().y() * 0.71f;
				ed->setSize(height * 4.9f, height);
				row.addElement(ed, false, true);
				break;
			}
		case MD_DATE:
			{
				ed = std::make_shared<DateTimeComponent>(window);
				row.addElement(ed, false);

				auto spacer = std::make_shared<GuiComponent>(mWindow);
				spacer->setSize(Renderer::getScreenWidth() * 0.0025f, 0);
				row.addElement(spacer, false);

				break;
			}
		case MD_TIME:
			{
				ed = std::make_shared<DateTimeComponent>(window, DateTimeComponent::DISP_RELATIVE_TO_NOW);
				row.addElement(ed, false);
				break;
			}
		case MD_MULTILINE_STRING:
		default:
			{
				// MD_STRING
				ed = std::make_shared<TextComponent>(window, "", Font::get(FONT_SIZE_SMALL, FONT_PATH_LIGHT), 0x777777FF, TextComponent::ALIGN_RIGHT);
				row.addElement(ed, true);
				
				auto spacer = std::make_shared<GuiComponent>(mWindow);
				spacer->setSize(Renderer::getScreenWidth() * 0.005f, 0);
				row.addElement(spacer, false);

				auto bracket = std::make_shared<ImageComponent>(mWindow);
				bracket->setImage(":/arrow.svg");
				bracket->setResize(Eigen::Vector2f(0, lbl->getFont()->getLetterHeight()));
				row.addElement(bracket, false);

				bool multiLine = iter->type == MD_MULTILINE_STRING;
				const std::string title = "INPUT GAME " + iter->key;
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
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "SCRAPE", "scrape", std::bind(&GuiMetaDataEd::fetch, this)));
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "SAVE", "save", [&] { save(); delete this; }));

	if(mDeleteFunc)
	{
		auto deleteFileAndSelf = [&] { mDeleteFunc(); delete this; };
		auto deleteBtnFunc = [this, deleteFileAndSelf] { mWindow->pushGui(new GuiMsgBox(mWindow, "This will delete a file!\nAre you sure?", "YES", deleteFileAndSelf, "NO", nullptr)); };
		buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "DELETE", "delete", deleteBtnFunc));
	}

	mButtons = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtons, Vector2i(0, 2), true, false);

	// initially put cursor on "SCRAPE"
	mGrid.setCursorTo(mButtons);
	
	// resize + center
	setSize(Renderer::getScreenWidth() * 0.5f, Renderer::getScreenHeight() * 0.71f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiMetaDataEd::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	mGrid.setSize(mSize);

	const float titleHeight = mTitle->getFont()->getLetterHeight();
	const float subtitleHeight = mSubtitle->getFont()->getLetterHeight() + 2;
	const float titleSubtitleSpacing = mSize.y() * 0.03f;

	mGrid.setRowHeightPerc(0, (titleHeight + titleSubtitleSpacing + subtitleHeight + TITLE_VERT_PADDING) / mSize.y());
	mGrid.setRowHeightPerc(2, mButtons->getSize().y() / mSize.y());

	mHeaderGrid->setRowHeightPerc(1, titleHeight / mHeaderGrid->getSize().y());
	mHeaderGrid->setRowHeightPerc(2, titleSubtitleSpacing / mHeaderGrid->getSize().y());
	mHeaderGrid->setRowHeightPerc(3, subtitleHeight / mHeaderGrid->getSize().y());
}

void GuiMetaDataEd::save()
{
	for(unsigned int i = 0; i < mEditors.size(); i++)
	{
		if(mMetaDataDecl.at(i).isStatistic)
			continue;

		mMetaData->set(mMetaDataDecl.at(i).key, mEditors.at(i)->getValue());
	}

	if(mSavedCallback)
		mSavedCallback();
}

void GuiMetaDataEd::fetch()
{
	GuiGameScraper* scr = new GuiGameScraper(mWindow, mScraperParams, std::bind(&GuiMetaDataEd::fetchDone, this, std::placeholders::_1));
	mWindow->pushGui(scr);
}

void GuiMetaDataEd::fetchDone(const ScraperSearchResult& result)
{
	for(unsigned int i = 0; i < mEditors.size(); i++)
	{
		if(mMetaDataDecl.at(i).isStatistic)
			continue;

		const std::string& key = mMetaDataDecl.at(i).key;
		mEditors.at(i)->setValue(result.mdl.get(key));
	}
}


bool GuiMetaDataEd::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if(input.value != 0 && config->isMappedTo("b", input))
	{
		delete this;
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiMetaDataEd::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mGrid.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "discard"));
	return prompts;
}
