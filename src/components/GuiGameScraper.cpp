#include "GuiGameScraper.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../scrapers/Scraper.h"
#include "../Settings.h"

GuiGameScraper::GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(MetaDataList)> doneFunc, std::function<void()> skipFunc) : GuiComponent(window), 
	mList(window, Eigen::Vector2i(2, 7 + MAX_SCRAPER_RESULTS)), 
	mBox(window, ":/frame.png"),
	mHeader(window, params.game->getBaseName(), Font::get(FONT_SIZE_MEDIUM)),
	mResultName(window, "", Font::get(FONT_SIZE_MEDIUM)),
	mResultInfo(window),
	mResultDesc(window, "", Font::get(FONT_SIZE_SMALL)),
	mResultThumbnail(window), 

	mSearchLabel(window, "Search for: ", Font::get(FONT_SIZE_SMALL)),
	mSearchText(window),

	mSearchParams(params),
	mDoneFunc(doneFunc),
	mSkipFunc(skipFunc)
{
	//				  FILE NAME
	//--------------------------------------
	//Name.................	|
	//Desc................. |   PREVIEW
	//..................... |    IMAGE?
	//....(autoscroll)..... |
	//--------------------------------------
	//Search for:  [_______________________] 
	//--------------------------------------
	//Result #1 Name
	//Result #2 Name
	//Result #3 Name
	//Result #4 Name
	//Result #5 Name

	addChild(&mBox);
	addChild(&mList);

	float sw = (float)Renderer::getScreenWidth();
	float sh = (float)Renderer::getScreenHeight();

	float colWidth = sw * 0.35f;

	mList.forceColumnWidth(0, (unsigned int)colWidth);
	mList.forceColumnWidth(1, (unsigned int)colWidth);

	using namespace Eigen;
	
	mList.setEntry(Vector2i(0, 0), Vector2i(2, 1), &mHeader, false, ComponentListComponent::AlignCenter);

	//y = 1 is a spacer row

	mResultName.setText(params.game->getName());
	mResultName.setColor(0x3B56CCFF);
	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), &mResultName, false, ComponentListComponent::AlignLeft);

	mResultDesc.setText(params.game->metadata()->get("desc"));
	mResultDesc.setSize(colWidth, 0);
	mResultInfo.addChild(&mResultDesc);
	mResultInfo.setSize(mResultDesc.getSize().x(), mResultDesc.getFont()->getHeight() * 3.0f);
	mList.setEntry(Vector2i(0, 2), Vector2i(1, 1), &mResultInfo, false, ComponentListComponent::AlignLeft);
	
	mResultThumbnail.setOrigin(0.5f, 0.5f);
	mResultThumbnail.setResize(colWidth, mResultInfo.getSize().y(), false);
	mList.setEntry(Vector2i(1, 2), Vector2i(1, 1), &mResultThumbnail, false, ComponentListComponent::AlignCenter);

	//y = 3 is a spacer row

	mList.setEntry(Vector2i(0, 4), Vector2i(1, 1), &mSearchLabel, false, ComponentListComponent::AlignLeft);
	mSearchText.setValue(!params.nameOverride.empty() ? params.nameOverride : params.game->getCleanName());
	mSearchText.setSize(colWidth * 2 - mSearchLabel.getSize().x() - 20, mSearchText.getSize().y());
	mList.setEntry(Vector2i(1, 4), Vector2i(1, 1), &mSearchText, true, ComponentListComponent::AlignRight);

	//y = 5 is a spacer row

	std::shared_ptr<Font> font = Font::get(FONT_SIZE_SMALL);
	mResultNames.reserve(MAX_SCRAPER_RESULTS);
	for(int i = 0; i < MAX_SCRAPER_RESULTS; i ++)
	{
		mResultNames.push_back(TextComponent(mWindow, "RESULT...", font));
		mResultNames.at(i).setColor(0x111111FF);
		mList.forceRowHeight(6 + i, (unsigned int)mResultNames.at(i).getSize().y());
	}

	mList.setPosition((sw - mList.getSize().x()) / 2, (sh - mList.getSize().y()) / 2);

	mBox.fitTo(mList.getSize(), mList.getPosition());

	mResultInfo.setAutoScroll(2200, 0.015f);

	mList.resetCursor();
}

void GuiGameScraper::search()
{
	//update mSearchParams
	mSearchParams.nameOverride = mSearchText.getValue();

	Settings::getInstance()->getScraper()->getResultsAsync(mSearchParams, mWindow, std::bind(&GuiGameScraper::onSearchDone, this, std::placeholders::_1));
}

void GuiGameScraper::onSearchDone(std::vector<MetaDataList> results)
{
	mList.removeEntriesIn(Eigen::Vector2i(0, 6), Eigen::Vector2i(1, 5));

	mScraperResults = results;

	const int end = results.size() > 5 ? 5 : results.size(); //at max display 5

	if(end == 0)
	{
		mResultNames.at(0).setText("No games found!");
		mList.setEntry(Eigen::Vector2i(0, 6), Eigen::Vector2i(1, 1), &mResultNames.at(0), false, ComponentListComponent::AlignLeft);
	}else{
		for(int i = 0; i < end; i++)
		{
			mResultNames.at(i).setText(results.at(i).get("name"));
			mList.setEntry(Eigen::Vector2i(0, 6 + i), Eigen::Vector2i(1, 1), &mResultNames.at(i), true, ComponentListComponent::AlignLeft);
		}
	}
}

int GuiGameScraper::getSelectedIndex()
{
	int index = 0;
	for(auto iter = mResultNames.begin(); iter != mResultNames.end(); iter++, index++)
	{
		if(&(*iter) == mList.getSelectedComponent())
			return index;
	}

	return -1;
}

bool GuiGameScraper::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		//if you're on a result
		if(getSelectedIndex() != -1)
		{
			mDoneFunc(mScraperResults.at(getSelectedIndex()));
			delete this;
			return true;
		}
	}else if(config->isMappedTo("b", input) && input.value != 0)
	{
		if(mSkipFunc)
			mSkipFunc();
		delete this;
		return true;
	}

	bool wasEditing = mSearchText.isEditing();
	bool ret = GuiComponent::input(config, input);

	if(config->isMappedTo("up", input) || config->isMappedTo("down", input) && input.value != 0)
	{
		//update game info pane
		int i = getSelectedIndex();
		if(i != -1)
		{
			mResultName.setText(mScraperResults.at(i).get("name"));
			mResultDesc.setText(mScraperResults.at(i).get("desc"));
			mResultInfo.setScrollPos(Eigen::Vector2d(0, 0));
			mResultInfo.resetAutoScrollTimer();

			std::string thumb = mScraperResults.at(i).get("thumbnail");
			mResultThumbnail.setImage("");
			if(!thumb.empty())
				mThumbnailReq = std::unique_ptr<HttpReq>(new HttpReq(thumb));
			else
				mThumbnailReq.reset();
		}
	}

	//stopped editing
	if(wasEditing && !mSearchText.isEditing())
	{
		//update results
		search();
	}

	return ret;
}

void GuiGameScraper::update(int deltaTime)
{
	if(mThumbnailReq && mThumbnailReq->status() != HttpReq::REQ_IN_PROGRESS)
	{
		updateThumbnail();
	}

	GuiComponent::update(deltaTime);
}

void GuiGameScraper::updateThumbnail()
{
	if(mThumbnailReq && mThumbnailReq->status() == HttpReq::REQ_SUCCESS)
	{
		std::string content = mThumbnailReq->getContent();
		mResultThumbnail.setImage(content.data(), content.length());
	}else{
		LOG(LogWarning) << "thumbnail req failed: " << mThumbnailReq->getErrorMsg();
		mResultThumbnail.setImage("");
	}

	mThumbnailReq.reset();
}
