#include "GuiMetaDataEd.h"
#include "../Renderer.h"
#include "../Log.h"
#include "AsyncReqComponent.h"
#include "../Settings.h"
#include "GuiGameScraper.h"
#include <boost/filesystem.hpp>

#define MDED_RESERVED_ROWS 3

GuiMetaDataEd::GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams scraperParams, 
	const std::string& header, std::function<void()> saveCallback, std::function<void()> deleteFunc) : GuiComponent(window), 
	mScraperParams(scraperParams), 
	mBox(mWindow, ":/frame.png", 0xAAAAAAFF, 0xCCCCCCFF),
	mList(window, Eigen::Vector2i(2, mdd.size() + MDED_RESERVED_ROWS)),
	mHeader(window),
	mMetaDataDecl(mdd), 
	mMetaData(md), 
	mSavedCallback(saveCallback), mDeleteFunc(deleteFunc), 
	mDeleteButton(window), mFetchButton(window), mSaveButton(window)
{
	unsigned int sw = Renderer::getScreenWidth();
	unsigned int sh = Renderer::getScreenHeight();
	
	addChild(&mBox);
	
	addChild(&mHeader);
	mHeader.setText(header);

	//initialize buttons
	mDeleteButton.setText("DELETE", mDeleteFunc ? 0xFF0000FF : 0x555555FF);
	if(mDeleteFunc)
		mDeleteButton.setPressedFunc([&] { mDeleteFunc(); delete this; });

	mFetchButton.setText("FETCH", 0x00FF00FF);
	mFetchButton.setPressedFunc(std::bind(&GuiMetaDataEd::fetch, this));

	mSaveButton.setText("SAVE", 0x0000FFFF);
	mSaveButton.setPressedFunc([&] { save(); delete this; });

	//initialize metadata list
	addChild(&mList);
	populateList(mdd);
	mList.setPosition((sw - mList.getSize().x()) / 2.0f, (sh - mList.getSize().y()) / 2.0f); //center it
	mList.resetCursor();

	mBox.fitTo(mList.getSize(), mList.getPosition(), Eigen::Vector2f(12, 12));

	mHeader.setPosition(mList.getPosition());
	mHeader.setSize(mList.getSize().x(), 0);
	mHeader.setCentered(true);
}

GuiMetaDataEd::~GuiMetaDataEd()
{
	for(auto iter = mLabels.begin(); iter != mLabels.end(); iter++)
	{
		delete *iter;
	}
	for(auto iter = mEditors.begin(); iter != mEditors.end(); iter++)
	{
		delete *iter;
	}
}

void GuiMetaDataEd::populateList(const std::vector<MetaDataDecl>& mdd)
{
	//      PATH		//(centered, not part of componentlist)

	//---GAME NAME---   //(at 1,1; size 3,1) (TextEditComponent)
	//DEL   SCRP  ---   //(buttons)
	//Fav:  Y/N   ---	//metadata start
	//Desc: ...   ---	//multiline texteditcomponent
	//Img:  ...   ---
	//---   SAVE  ---

	using namespace Eigen;

	int y = 0;

	//fetch button
	mList.setEntry(Vector2i(0, y), Vector2i(1, 1), &mFetchButton, true, ComponentListComponent::AlignLeft);

	//delete button
	mList.setEntry(Vector2i(1, y), Vector2i(1, 1), &mDeleteButton, true, ComponentListComponent::AlignRight);

	y++;

	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		if (!iter->isInternal) {
			TextComponent* label = new TextComponent(mWindow);
			label->setText(iter->key);
			mList.setEntry(Vector2i(0, y), Vector2i(1, 1), label, false, ComponentListComponent::AlignLeft);
			mLabels.push_back(label);

			GuiComponent* ed = MetaDataList::makeEditor(mWindow, iter->type);
			ed->setSize(Renderer::getScreenWidth() * 0.4f, ed->getSize().y());
			ed->setValue(mMetaData->get(iter->key));
			mList.setEntry(Vector2i(1, y), Vector2i(1, 1), ed, true, ComponentListComponent::AlignRight);
			mEditors.push_back(ed);

			y++;
		}
	}

	//save button
	mList.setEntry(Vector2i(0, y), Vector2i(2, 1), &mSaveButton, true, ComponentListComponent::AlignCenter);
}

void GuiMetaDataEd::save()
{
	for(unsigned int i = 0; i < mLabels.size(); i++)
	{
		mMetaData->set(mLabels.at(i)->getValue(), mEditors.at(i)->getValue());
	}

	if(mSavedCallback)
		mSavedCallback();
}

void GuiMetaDataEd::fetch()
{
	GuiGameScraper* scr = new GuiGameScraper(mWindow, mScraperParams, std::bind(&GuiMetaDataEd::fetchDone, this, std::placeholders::_1));
	mWindow->pushGui(scr);
	scr->search();
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
