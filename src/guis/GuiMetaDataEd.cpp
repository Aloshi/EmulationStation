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

GuiMetaDataEd::GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams scraperParams, 
	const std::string& header, std::function<void()> saveCallback, std::function<void()> deleteFunc) : GuiComponent(window), 
	mScraperParams(scraperParams), 
	mMenu(window, header.c_str()), 
	mMetaDataDecl(mdd), 
	mMetaData(md), 
	mSavedCallback(saveCallback), mDeleteFunc(deleteFunc)
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	
	addChild(&mMenu);

	// populate list
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		std::shared_ptr<GuiComponent> ed;

		// create ed and add it (and any related components) to mMenu
		// ed's value will be set below
		ComponentListRow row;
		auto lbl = std::make_shared<TextComponent>(mWindow, strToUpper(iter->key), Font::get(FONT_SIZE_SMALL), 0x777777FF);
		row.addElement(lbl, true); // label

		switch(iter->type)
		{
		case MD_RATING:
			{
				ed = std::make_shared<RatingComponent>(window);
				ed->setSize(0, lbl->getSize().y());
				row.addElement(ed, false, true);
				mMenu.addRow(row);
				break;
			}
		case MD_DATE:
			{
				ed = std::make_shared<DateTimeComponent>(window);
				row.addElement(ed, false);
				mMenu.addRow(row);
				break;
			}
		case MD_TIME:
			{
				ed = std::make_shared<DateTimeComponent>(window, DateTimeComponent::DISP_RELATIVE_TO_NOW);
				row.addElement(ed, false);
				mMenu.addRow(row);
				break;
			}
		case MD_MULTILINE_STRING:
		default:
			{
				// MD_STRING
				ed = std::make_shared<TextComponent>(window, "", Font::get(FONT_SIZE_SMALL, FONT_PATH_LIGHT), 0x777777FF, TextComponent::ALIGN_RIGHT);
				row.addElement(ed, true);
				
				auto bracket = std::make_shared<ImageComponent>(mWindow);
				bracket->setImage(":/arrow.svg");
				bracket->setResize(Eigen::Vector2f(0, lbl->getSize().y() * 0.8f));
				row.addElement(bracket, false);

				bool multiLine = iter->type == MD_MULTILINE_STRING;
				const std::string& title = iter->key;
				auto updateVal = [ed](const std::string& newVal) { ed->setValue(newVal); }; // ok callback (apply new value to ed)
				row.makeAcceptInputHandler([this, title, ed, updateVal, multiLine] {
					mWindow->pushGui(new GuiTextEditPopup(mWindow, title, ed->getValue(), updateVal, multiLine));
				});

				mMenu.addRow(row);
				break;
			}
		}

		assert(ed);
		ed->setValue(mMetaData->get(iter->key));
		mEditors.push_back(ed);
	}

	//add buttons	
	mMenu.addButton("SCRAPE", "download metadata from the Internet", std::bind(&GuiMetaDataEd::fetch, this));
	mMenu.addButton("SAVE", "save changes", [&] { save(); delete this; });
	
	if(mDeleteFunc)
	{
		auto deleteFileAndSelf = [&] { mDeleteFunc(); delete this; };
		auto deleteBtnFunc = [this, deleteFileAndSelf] { mWindow->pushGui(new GuiMsgBox(mWindow, "This will delete a file!\nAre you sure?", "YES", deleteFileAndSelf, "NO", nullptr)); };
		mMenu.addButton("DELETE", "delete this game on disk", deleteBtnFunc);
	}

	// initially put cursor on "SCRAPE"
	mMenu.setCursorToButtons();

	// position menu
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f); //center it
}

void GuiMetaDataEd::save()
{
	for(unsigned int i = 0; i < mEditors.size(); i++)
	{
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
		//don't overwrite statistics
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
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "discard changes"));
	return prompts;
}
