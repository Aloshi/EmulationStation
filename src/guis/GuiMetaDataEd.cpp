#include "GuiMetaDataEd.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../components/AsyncReqComponent.h"
#include "../Settings.h"
#include "GuiGameScraper.h"
#include "GuiMsgBox.h"
#include <boost/filesystem.hpp>

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
		auto ed = MetaDataList::makeEditor(mWindow, iter->type);
		ed->setValue(mMetaData->get(iter->key));
		mEditors.push_back(ed);
		mMenu.addWithLabel(iter->key, ed);
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

void GuiMetaDataEd::fetchDone(MetaDataList result)
{
	//TODO - replace this with resolveMetaDataAssetsAsync!

	//this is a little tricky:
	//go through the list of returned results, if anything is an image and the path looks like a URL:
	//  (1) start an async download + resize (will create an AsyncReq that blocks further user input)
	//      (when this is finished, call result.set(key, newly_downloaded_file_path) and call fetchDone() again)
	//  (2) return from this function immediately
	for(auto it = mMetaDataDecl.begin(); it != mMetaDataDecl.end(); it++)
	{
		std::string key = it->key;
		std::string val = result.get(it->key);

		//val is /probably/ a URL
		if(it->type == MD_IMAGE_PATH && HttpReq::isUrl(val))
		{
			downloadImageAsync(mWindow, val, getSaveAsPath(mScraperParams, key, val), 
				[this, result, key] (std::string filePath) mutable -> void {
					//skip it
					if(filePath.empty())
						LOG(LogError) << "Error resolving boxart";

					result.set(key, filePath);
					this->fetchDone(result);
			});
			return;
		}
	}

	for(unsigned int i = 0; i < mEditors.size(); i++)
	{
		//don't overwrite statistics
		if(mMetaDataDecl.at(i).isStatistic)
			continue;

		const std::string& key = mMetaDataDecl.at(i).key;
		mEditors.at(i)->setValue(result.get(key));
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
