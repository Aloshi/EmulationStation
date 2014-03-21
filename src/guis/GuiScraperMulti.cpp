#include "GuiScraperMulti.h"
#include "../Renderer.h"

#include "../components/TextComponent.h"
#include "../components/ButtonComponent.h"
#include "../components/ScraperSearchComponent.h"
#include "../components/MenuComponent.h" // for makeButtonGrid
#include "GuiMsgBox.h"

using namespace Eigen;

GuiScraperMulti::GuiScraperMulti(Window* window, const std::queue<ScraperSearchParams>& searches, bool approveResults) : 
	GuiComponent(window), mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 4)), 
	mSearchQueue(searches)
{
	addChild(&mBackground);
	addChild(&mGrid);

	mTotalGames = mSearchQueue.size();
	mCurrentGame = 0;

	// set up grid
	mTitle = std::make_shared<TextComponent>(mWindow, "SCRAPING IN PROGRESS", Font::get(FONT_SIZE_SMALL), 0x777777FF, true);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

	mSubtitle = std::make_shared<TextComponent>(mWindow, "subtitle text", Font::get(FONT_SIZE_SMALL), 0x888888FF, true);
	mGrid.setEntry(mSubtitle, Vector2i(0, 1), false, true);

	mSearchComp = std::make_shared<ScraperSearchComponent>(mWindow, 
		approveResults ? ScraperSearchComponent::ALWAYS_ACCEPT_MATCHING_CRC : ScraperSearchComponent::ALWAYS_ACCEPT_FIRST_RESULT);
	mSearchComp->setAcceptCallback(std::bind(&GuiScraperMulti::acceptResult, this, std::placeholders::_1));
	mSearchComp->setCancelCallback(std::bind(&GuiScraperMulti::finish, this));
	mGrid.setEntry(mSearchComp, Vector2i(0, 2), approveResults, true);

	std::vector< std::shared_ptr<ButtonComponent> > buttons;
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "INPUT", "manually search by name", nullptr));
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "SKIP", "skip this game", std::bind(&GuiScraperMulti::skip, this)));
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "STOP", "cancel scraping", std::bind(&GuiScraperMulti::finish, this)));
	mButtonGrid = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtonGrid, Vector2i(0, 3), true, false);

	setSize(Renderer::getScreenWidth() * 0.7f, Renderer::getScreenHeight() * 0.65f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);

	doNextSearch();
}

void GuiScraperMulti::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));
	mGrid.setSize(mSize);

	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mGrid.getSize().y());
	mGrid.setRowHeightPerc(1, mSubtitle->getFont()->getHeight() / mGrid.getSize().y());
	mGrid.setRowHeightPerc(3, mButtonGrid->getSize().y() / mGrid.getSize().y());
}

void GuiScraperMulti::doNextSearch()
{
	if(mSearchQueue.empty())
	{
		finish();
		return;
	}

	// update subtitle
	std::stringstream ss;
	ss << "GAME " << (mCurrentGame + 1) << " OF " << mTotalGames;
	mSubtitle->setText(ss.str());

	mSearchComp->search(mSearchQueue.front());
}

void GuiScraperMulti::acceptResult(const ScraperSearchResult& result)
{
	ScraperSearchParams& search = mSearchQueue.front();

	search.game->metadata = result.mdl;

	mSearchQueue.pop();
	mCurrentGame++;
	doNextSearch();
}

void GuiScraperMulti::skip()
{
	mSearchQueue.pop();
	mCurrentGame++;
	doNextSearch();
}

void GuiScraperMulti::finish()
{
	mWindow->pushGui(new GuiMsgBox(mWindow, "SCRAPING COMPLETE!", 
		"OK", [&] { delete this; }));
}

std::vector<HelpPrompt> GuiScraperMulti::getHelpPrompts()
{
	return mGrid.getHelpPrompts();
}
