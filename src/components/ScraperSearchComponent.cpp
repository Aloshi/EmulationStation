#include "ScraperSearchComponent.h"

#include "../components/TextComponent.h"
#include "../components/ScrollableContainer.h"
#include "../components/ImageComponent.h"
#include "../components/ComponentList.h"
#include "../HttpReq.h"
#include "../Settings.h"
#include "../Log.h"

ScraperSearchComponent::ScraperSearchComponent(Window* window, SearchType type) : GuiComponent(window),
	mGrid(window, Eigen::Vector2i(4, 3)),
	mSearchType(type)
{
	mSearchParams.system = NULL;
	mSearchParams.game = NULL;

	addChild(&mGrid);

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

	mResultThumbnail->setMaxSize(mGrid.getColWidth(1), mGrid.getRowHeight(1));
	mResultDesc->setSize(mDescContainer->getSize().x(), 0); // make desc text wrap at edge of container
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

void ScraperSearchComponent::setSearchParams(const ScraperSearchParams& params)
{
	mSearchParams = params;
	search();
}

void ScraperSearchComponent::search()
{
	Settings::getInstance()->getScraper()->getResultsAsync(mSearchParams, mWindow, std::bind(&ScraperSearchComponent::onSearchReceived, this, std::placeholders::_1));
}

void ScraperSearchComponent::onSearchReceived(std::vector<MetaDataList> results)
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
			row.addElement(std::make_shared<TextComponent>(mWindow, results.at(i).get("name"), font, color), true);
			mResultList->addRow(row);
		}
		mGrid.resetCursor();
	}

	updateInfoPane();

	if(mSearchType == ALWAYS_ACCEPT_FIRST_RESULT)
	{
		if(mScraperResults.size() == 0)
			returnResult(NULL);
		else
			returnResult(&mScraperResults.front());
	}else if(mSearchType == ALWAYS_ACCEPT_MATCHING_CRC)
	{
		// TODO
		assert(false);
	}
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
		mResultName->setText(mScraperResults.at(i).get("name"));
		mResultDesc->setText(mScraperResults.at(i).get("desc"));
		mDescContainer->setScrollPos(Eigen::Vector2d(0, 0));
		mDescContainer->resetAutoScrollTimer();

		std::string thumb = mScraperResults.at(i).get("thumbnail");
		mResultThumbnail->setImage("");
		if(!thumb.empty())
			mThumbnailReq = std::unique_ptr<HttpReq>(new HttpReq(thumb));
		else
			mThumbnailReq.reset();
	}
}

bool ScraperSearchComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		//if you're on a result
		if(getSelectedIndex() != -1)
		{
			returnResult(&mScraperResults.at(getSelectedIndex()));
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

void ScraperSearchComponent::returnResult(MetaDataList* result)
{
	assert(mAcceptCallback);
	mAcceptCallback(result);
}

void ScraperSearchComponent::update(int deltaTime)
{
	if(mThumbnailReq && mThumbnailReq->status() != HttpReq::REQ_IN_PROGRESS)
	{
		updateThumbnail();
	}

	GuiComponent::update(deltaTime);
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