#include "GuiGameScraper.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../scrapers/Scraper.h"
#include "../Settings.h"

#include "../components/TextComponent.h"
#include "../components/ButtonComponent.h"

GuiGameScraper::GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(MetaDataList)> doneFunc, std::function<void()> skipFunc) : GuiComponent(window), 
	mGrid(window, Eigen::Vector2i(1, 3)), 
	mBox(window, ":/frame.png"),
	mSearchParams(params),
	mDoneFunc(doneFunc),
	mSkipFunc(skipFunc),
	mSearchCountdown(2)
{
	// new screen:
	
	//				  FILE NAME
	//--------------------------------------
	//   Result Title     |  Result #1
	// |-------|  .....   |  Result #2
	// |  IMG  |  info    |  Result #3
	// |-------|  .....   |  .........
	//                    |  .........
	// DESCRIPTION........|  .........
	// ...................|  .........
	// ...................|  .........
	//--------------------------------------
	//       [SEARCH NAME] [CANCEL]


	addChild(&mBox);
	addChild(&mGrid);

	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	mGrid.setSize(mSize.x() * 0.7f, mSize.y() * 0.65f);
	
	auto headerFont = Font::get(FONT_SIZE_LARGE);

	mGrid.setRowHeightPerc(0, headerFont->getHeight() / mGrid.getSize().y()); // header
	mGrid.setRowHeightPerc(2, 0.19f); // buttons

	// header
	mGrid.setEntry(std::make_shared<TextComponent>(mWindow, getCleanFileName(mSearchParams.game->getName()), 
		headerFont, 0x777777FF, true), Eigen::Vector2i(0, 0), false, true);

	// ScraperSearchComponent
	mSearch = std::make_shared<ScraperSearchComponent>(window, ScraperSearchComponent::NEVER_AUTO_ACCEPT);
	mGrid.setEntry(mSearch, Eigen::Vector2i(0, 1), true);

	// buttons
	auto buttonGrid = std::make_shared<ComponentGrid>(mWindow, Eigen::Vector2i(3, 1));
	
	auto manualSearchBtn = std::make_shared<ButtonComponent>(mWindow, "MANUAL SEARCH", "enter search terms");
	auto cancelBtn = std::make_shared<ButtonComponent>(mWindow, "CANCEL", "cancel");

	buttonGrid->setSize(manualSearchBtn->getSize().x() + cancelBtn->getSize().x() + 18, manualSearchBtn->getSize().y());
	buttonGrid->setColWidthPerc(0, 0.5f);
	buttonGrid->setColWidthPerc(1, 0.5f);

	buttonGrid->setEntry(manualSearchBtn, Eigen::Vector2i(0, 0), true, false);
	buttonGrid->setEntry(cancelBtn, Eigen::Vector2i(1, 0), true, false);

	mGrid.setEntry(buttonGrid, Eigen::Vector2i(0, 2), true, false);

	// center everything
	mGrid.setPosition((mSize.x() - mGrid.getSize().x()) / 2, (mSize.y() - mGrid.getSize().y()) / 2);
	mBox.fitTo(mGrid.getSize(), mGrid.getPosition(), Eigen::Vector2f(-32, -32));

	mSearch->setAcceptCallback( [this](MetaDataList* result) { 
		if(result != NULL)
			this->mDoneFunc(*result);
		else if(this->mSkipFunc)
			this->mSkipFunc();

		delete this;
	});

	mGrid.resetCursor();
	//mSearch->setSearchParams(params); // also starts the search
}

bool GuiGameScraper::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("b", input) && input.value)
	{
		if(mSkipFunc)
			mSkipFunc();
		delete this;
		return true;
	}

	return GuiComponent::input(config, input);
}

void GuiGameScraper::update(int deltaTime)
{
	// HAAACK because AsyncReq wont get pushed in the right order if search happens on creation
	if(mSearchCountdown > 0)
	{
		mSearchCountdown--;
		if(mSearchCountdown == 0)
			mSearch->setSearchParams(mSearchParams);
	}

	GuiComponent::update(deltaTime);
}

std::vector<HelpPrompt> GuiGameScraper::getHelpPrompts()
{
	return mGrid.getHelpPrompts();
}
