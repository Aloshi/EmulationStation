#include "GuiGameScraper.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../scrapers/Scraper.h"
#include "../Settings.h"

#include "../components/TextComponent.h"
#include "../components/ButtonComponent.h"
#include "../components/MenuComponent.h"

GuiGameScraper::GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(const ScraperSearchResult&)> doneFunc) : GuiComponent(window), 
	mGrid(window, Eigen::Vector2i(1, 3)), 
	mBox(window, ":/frame.png"),
	mSearchParams(params),
	mClose(false)
{
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
	std::vector< std::shared_ptr<ButtonComponent> > buttons;
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "INPUT", "manually search"));
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "CANCEL", "cancel", [&] { delete this; }));
	auto buttonGrid = makeButtonGrid(mWindow, buttons);

	mGrid.setEntry(buttonGrid, Eigen::Vector2i(0, 2), true, false);

	// center everything
	mGrid.setPosition((mSize.x() - mGrid.getSize().x()) / 2, (mSize.y() - mGrid.getSize().y()) / 2);
	mBox.fitTo(mGrid.getSize(), mGrid.getPosition(), Eigen::Vector2f(-32, -32));

	// we call this->close() instead of just delete this; in the accept callback:
	// this is because of how GuiComponent::update works.  if it was just delete this, this would happen when the metadata resolver is done:
	//     GuiGameScraper::update()
	//       GuiComponent::update()
	//         it = mChildren.begin();
	//         mBox::update()
	//         it++;
	//         mSearchComponent::update()
	//           acceptCallback -> delete this
	//         it++; // error, mChildren has been deleted because it was part of this

	// so instead we do this:
	//     GuiGameScraper::update()
	//       GuiComponent::update()
	//         it = mChildren.begin();
	//         mBox::update()
	//         it++;
	//         mSearchComponent::update()
	//           acceptCallback -> close() -> mClose = true
	//         it++; // ok
	//       if(mClose)
	//         delete this;
	mSearch->setAcceptCallback([this, doneFunc](const ScraperSearchResult& result) { doneFunc(result); close(); });
	mSearch->setCancelCallback([&] { delete this; });

	mGrid.resetCursor();
	mSearch->search(params); // start the search
}

bool GuiGameScraper::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("b", input) && input.value)
	{
		delete this;
		return true;
	}

	return GuiComponent::input(config, input);
}

void GuiGameScraper::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	if(mClose)
		delete this;
}

std::vector<HelpPrompt> GuiGameScraper::getHelpPrompts()
{
	return mGrid.getHelpPrompts();
}

void GuiGameScraper::close()
{
	mClose = true;
}