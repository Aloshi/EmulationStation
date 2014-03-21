#include "ScraperSearchComponent.h"

#include "../guis/GuiMsgBox.h"
#include "TextComponent.h"
#include "ScrollableContainer.h"
#include "ImageComponent.h"
#include "RatingComponent.h"
#include "DateTimeComponent.h"
#include "ComponentList.h"
#include "../HttpReq.h"
#include "../Settings.h"
#include "../Log.h"
#include "../Util.h"
#include "../guis/GuiTextEditPopup.h"

ScraperSearchComponent::ScraperSearchComponent(Window* window, SearchType type) : GuiComponent(window),
	mGrid(window, Eigen::Vector2i(4, 3)),
	mSearchType(type)
{
	addChild(&mGrid);

	mBlockAccept = false;

	using namespace Eigen;

	// left spacer (empty component, needed for borders)
	mGrid.setEntry(std::make_shared<GuiComponent>(mWindow), Vector2i(0, 0), false, false, Vector2i(1, 3), GridFlags::BORDER_TOP | GridFlags::BORDER_BOTTOM);

	// selected result name
	mResultName = std::make_shared<TextComponent>(mWindow, "Result name", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
	mGrid.setEntry(mResultName, Vector2i(1, 0), false, true, Vector2i(2, 1), GridFlags::BORDER_TOP);

	// selected result thumbnail
	mResultThumbnail = std::make_shared<ImageComponent>(mWindow);
	mGrid.setEntry(mResultThumbnail, Vector2i(1, 1), false, false, Vector2i(1, 1));

	// selected result desc + container
	mDescContainer = std::make_shared<ScrollableContainer>(mWindow);
	mResultDesc = std::make_shared<TextComponent>(mWindow, "Result desc", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	mDescContainer->addChild(mResultDesc.get());
	mDescContainer->setAutoScroll(2200, 0.015f);
	
	// metadata
	auto font = Font::get(FONT_SIZE_SMALL); // this gets replaced in onSizeChanged() so its just a placeholder
	const unsigned int mdColor = 0x777777FF;
	const unsigned int mdLblColor = 0x666666FF;
	mMD_Rating = std::make_shared<RatingComponent>(mWindow);
	mMD_ReleaseDate = std::make_shared<DateTimeComponent>(mWindow);
	mMD_ReleaseDate->setColor(mdColor);
	mMD_Developer = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
	mMD_Publisher = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
	mMD_Genre = std::make_shared<TextComponent>(mWindow, "", font, mdColor);
	mMD_Players = std::make_shared<TextComponent>(mWindow, "", font, mdColor);

	mMD_Pairs.push_back(MetaDataPair(std::make_shared<TextComponent>(mWindow, "RATING:", font, mdLblColor), mMD_Rating));
	mMD_Pairs.push_back(MetaDataPair(std::make_shared<TextComponent>(mWindow, "RELEASED:", font, mdLblColor), mMD_ReleaseDate));
	mMD_Pairs.push_back(MetaDataPair(std::make_shared<TextComponent>(mWindow, "DEVELOPER:", font, mdLblColor), mMD_Developer));
	mMD_Pairs.push_back(MetaDataPair(std::make_shared<TextComponent>(mWindow, "PUBLISHER:", font, mdLblColor), mMD_Publisher));
	mMD_Pairs.push_back(MetaDataPair(std::make_shared<TextComponent>(mWindow, "GENRE:", font, mdLblColor), mMD_Genre));
	mMD_Pairs.push_back(MetaDataPair(std::make_shared<TextComponent>(mWindow, "PLAYERS:", font, mdLblColor), mMD_Players));

	mMD_Grid = std::make_shared<ComponentGrid>(mWindow, Vector2i(2, mMD_Pairs.size()));
	unsigned int i = 0;
	for(auto it = mMD_Pairs.begin(); it != mMD_Pairs.end(); it++)
	{
		mMD_Grid->setEntry(it->first, Vector2i(0, i), false, true);
		mMD_Grid->setEntry(it->second, Vector2i(1, i), false, true);
		i++;
	}

	mGrid.setEntry(mMD_Grid, Vector2i(2, 1), false, true);

	// result list
	mResultList = std::make_shared<ComponentList>(mWindow);
	
	updateViewStyle();
}

void ScraperSearchComponent::onSizeChanged()
{
	mGrid.setSize(mSize);
	
	// column widths
	mGrid.setColWidthPerc(0, 0.01f);
	mGrid.setColWidthPerc(1, 0.25f);
	mGrid.setColWidthPerc(2, 0.25f);
	mGrid.setColWidthPerc(3, 0.49f);

	// row heights
	const float fontHeightPerc = (mResultName->getFont()->getHeight()) / mGrid.getSize().y();
	mGrid.setRowHeightPerc(0, fontHeightPerc); // result name
	mGrid.setRowHeightPerc(2, 0.375f); // description

	// limit thumbnail size using setMaxHeight - we do this instead of letting mGrid call setSize because it maintains the aspect ratio
	// we also pad a little so it doesn't rub up against the metadata labels
	mResultThumbnail->setMaxSize(mGrid.getColWidth(1) - 16, mGrid.getRowHeight(1));
	mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container

	// metadata
	// (mMD_Grid has already been resized by mGrid)

	if(mMD_Grid->getSize().y() > mMD_Pairs.size())
	{
		const int fontHeight = (int)(mMD_Grid->getSize().y() / mMD_Pairs.size() * 0.8f);
		auto fontLbl = Font::get(fontHeight, FONT_PATH_REGULAR);
		auto fontComp = Font::get(fontHeight, FONT_PATH_LIGHT);

		// update label fonts
		float maxLblWidth = 0;
		for(auto it = mMD_Pairs.begin(); it != mMD_Pairs.end(); it++)
		{
			it->first->setFont(fontLbl);
			it->first->setSize(0, 0);
			if(it->first->getSize().x() > maxLblWidth)
				maxLblWidth = it->first->getSize().x() + 6;
		}

		// update component fonts
		mMD_ReleaseDate->setFont(fontComp);
		mMD_Developer->setFont(fontComp);
		mMD_Publisher->setFont(fontComp);
		mMD_Genre->setFont(fontComp);
		mMD_Players->setFont(fontComp);

		mMD_Grid->setColWidthPerc(0, maxLblWidth / mMD_Grid->getSize().x());
	}
}

void ScraperSearchComponent::updateViewStyle()
{
	using namespace Eigen;

	// unlink description and result list
	mGrid.removeEntry(mResultDesc);
	mGrid.removeEntry(mResultList);

	// add them back depending on search type
	if(mSearchType == ALWAYS_ACCEPT_FIRST_RESULT)
	{
		// show description on the right
		mGrid.setEntry(mDescContainer, Vector2i(3, 0), false, true, Vector2i(1, 3), GridFlags::BORDER_TOP | GridFlags::BORDER_BOTTOM);
		mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container
	}else{
		// show result list on the right
		mGrid.setEntry(mResultList, Vector2i(3, 0), true, true, Vector2i(1, 3), GridFlags::BORDER_LEFT | GridFlags::BORDER_TOP | GridFlags::BORDER_BOTTOM);

		// show description under image/info
		mGrid.setEntry(mDescContainer, Vector2i(1, 2), false, true, Vector2i(2, 1), GridFlags::BORDER_BOTTOM);
		mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container
	}
}

void ScraperSearchComponent::search(const ScraperSearchParams& params)
{
	mResultList->clear();
	mScraperResults.clear();
	mThumbnailReq.reset();
	mMDResolveHandle.reset();
	updateInfoPane();

	mLastSearch = params;
	mSearchHandle = Settings::getInstance()->getScraper()->getResultsAsync(params);
}

void ScraperSearchComponent::stop()
{
	mThumbnailReq.reset();
	mSearchHandle.reset();
	mMDResolveHandle.reset();
	mBlockAccept = false;
}

void ScraperSearchComponent::onSearchDone(const std::vector<ScraperSearchResult>& results)
{
	mResultList->clear();

	mScraperResults = results;

	const int end = results.size() > 5 ? 5 : results.size(); // at max display 5

	auto font = Font::get(FONT_SIZE_MEDIUM);
	unsigned int color = 0x777777FF;
	if(end == 0)
	{
		ComponentListRow row;
		row.addElement(std::make_shared<TextComponent>(mWindow, "No games found!", font, color), true);
		mResultList->addRow(row);
		mGrid.resetCursor();
	}else{
		ComponentListRow row;
		for(int i = 0; i < end; i++)
		{
			row.elements.clear();
			row.addElement(std::make_shared<TextComponent>(mWindow, results.at(i).mdl.get("name"), font, color), true);
			mResultList->addRow(row);
		}
		mGrid.resetCursor();
	}

	mBlockAccept = false;
	updateInfoPane();

	if(mSearchType == ALWAYS_ACCEPT_FIRST_RESULT)
	{
		if(mScraperResults.size() == 0)
			mSkipCallback();
		else
			returnResult(mScraperResults.front());
	}else if(mSearchType == ALWAYS_ACCEPT_MATCHING_CRC)
	{
		// TODO
	}
}

void ScraperSearchComponent::onSearchError(const std::string& error)
{
	mWindow->pushGui(new GuiMsgBox(mWindow, error,
		"RETRY", std::bind(&ScraperSearchComponent::search, this, mLastSearch),
		"SKIP", mSkipCallback,
		"CANCEL", mCancelCallback));
}

int ScraperSearchComponent::getSelectedIndex()
{
	if(mScraperResults.size() && mGrid.getSelectedComponent() != mResultList)
		return -1;

	return mResultList->getCursorId();
}

void ScraperSearchComponent::updateInfoPane()
{
	int i = getSelectedIndex();
	if(i != -1 && (int)mScraperResults.size() > i)
	{
		mResultName->setText(mScraperResults.at(i).mdl.get("name"));
		mResultDesc->setText(mScraperResults.at(i).mdl.get("desc"));
		mDescContainer->setScrollPos(Eigen::Vector2d(0, 0));
		mDescContainer->resetAutoScrollTimer();

		mResultThumbnail->setImage("");
		const std::string& thumb = mScraperResults.at(i).thumbnailUrl;
		if(!thumb.empty())
			mThumbnailReq = std::unique_ptr<HttpReq>(new HttpReq(thumb));
		else
			mThumbnailReq.reset();

		// metadata
		mMD_Rating->setValue(strToUpper(mScraperResults.at(i).mdl.get("rating")));
		mMD_ReleaseDate->setValue(strToUpper(mScraperResults.at(i).mdl.get("releasedate")));
		mMD_Developer->setText(strToUpper(mScraperResults.at(i).mdl.get("developer")));
		mMD_Publisher->setText(strToUpper(mScraperResults.at(i).mdl.get("publisher")));
		mMD_Genre->setText(strToUpper(mScraperResults.at(i).mdl.get("genre")));
		mMD_Players->setText(strToUpper(mScraperResults.at(i).mdl.get("players")));

	}else{
		mResultName->setText("");
		mResultDesc->setText("");
		mResultThumbnail->setImage("");

		// metadata
		mMD_Rating->setValue("");
		mMD_ReleaseDate->setValue("");
		mMD_Developer->setText("");
		mMD_Publisher->setText("");
		mMD_Genre->setText("");
		mMD_Players->setText("");
	}
}

bool ScraperSearchComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		if(mBlockAccept)
			return true;

		//if you're on a result
		if(getSelectedIndex() != -1)
		{
			returnResult(mScraperResults.at(getSelectedIndex()));
			return true;
		}
	}

	bool ret = GuiComponent::input(config, input);

	if(config->isMappedTo("up", input) || config->isMappedTo("down", input) && input.value != 0)
	{
		updateInfoPane();
	}

	return ret;
}

void ScraperSearchComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	renderChildren(trans);

	if(mBlockAccept)
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect((int)mResultList->getPosition().x(), (int)mResultList->getPosition().y(),
			(int)mResultList->getSize().x(), (int)mResultList->getSize().y(), 0x00000011);
	}
}

void ScraperSearchComponent::returnResult(ScraperSearchResult result)
{
	mBlockAccept = true;

	// resolve metadata image before returning
	if(!result.imageUrl.empty())
	{
		mMDResolveHandle = resolveMetaDataAssets(result, mLastSearch);
		return;
	}

	mAcceptCallback(result);
}

void ScraperSearchComponent::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	if(mThumbnailReq && mThumbnailReq->status() != HttpReq::REQ_IN_PROGRESS)
	{
		updateThumbnail();
	}

	if(mSearchHandle && mSearchHandle->status() != ASYNC_IN_PROGRESS)
	{
		if(mSearchHandle->status() == ASYNC_DONE)
		{
			onSearchDone(mSearchHandle->getResults());
		}else if(mSearchHandle->status() == ASYNC_ERROR)
		{
			onSearchError(mSearchHandle->getStatusString());
		}
		mSearchHandle.reset();
	}

	if(mMDResolveHandle && mMDResolveHandle->status() != ASYNC_IN_PROGRESS)
	{
		if(mMDResolveHandle->status() == ASYNC_DONE)
		{
			ScraperSearchResult result = mMDResolveHandle->getResult();
			mMDResolveHandle.reset();

			// this might end in us being deleted, depending on mAcceptCallback - so make sure this is the last thing we do in update()
			returnResult(result);
		}else if(mMDResolveHandle->status() == ASYNC_ERROR)
		{
			onSearchError(mMDResolveHandle->getStatusString());
			mMDResolveHandle.reset();
		}
	}
}

void ScraperSearchComponent::updateThumbnail()
{
	if(mThumbnailReq && mThumbnailReq->status() == HttpReq::REQ_SUCCESS)
	{
		std::string content = mThumbnailReq->getContent();
		mResultThumbnail->setImage(content.data(), content.length());
	}else{
		LOG(LogWarning) << "thumbnail req failed: " << mThumbnailReq->getErrorMsg();
		mResultThumbnail->setImage("");
	}

	mThumbnailReq.reset();
	mGrid.onSizeChanged(); // a hack to fix the thumbnail position since its size changed
}

void ScraperSearchComponent::openInputScreen(ScraperSearchParams& params)
{
	auto searchForFunc = [&](const std::string& name)
	{
		params.nameOverride = name;
		search(params);
	};

	stop();
	mWindow->pushGui(new GuiTextEditPopup(mWindow, "SEARCH FOR", 
		// initial value is last search if there was one, otherwise the clean path name
		params.nameOverride.empty() ? getCleanFileName(params.game->getPath()) : params.nameOverride, 
		searchForFunc, false, "SEARCH"));
}

std::vector<HelpPrompt> ScraperSearchComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mGrid.getHelpPrompts();
	if(getSelectedIndex() != -1)
		prompts.push_back(HelpPrompt("a", "accept result"));
	
	return prompts;
}

void ScraperSearchComponent::onFocusGained()
{
	mGrid.onFocusGained();
}

void ScraperSearchComponent::onFocusLost()
{
	mGrid.onFocusLost();
}
