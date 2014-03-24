#include "GuiGameScraper.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../scrapers/Scraper.h"
#include "../Settings.h"

#include "../components/TextComponent.h"
#include "../components/ButtonComponent.h"
#include "../components/MenuComponent.h"
#include "GuiTextEditPopup.h"

GuiGameScraper::GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(const ScraperSearchResult&)> doneFunc) : GuiComponent(window), 
	mGrid(window, Eigen::Vector2i(1, 3)), 
	mBox(window, ":/frame.png"),
	mSearchParams(params),
	mClose(false)
{
	addChild(&mBox);
	addChild(&mGrid);

	// header
	mHeader = std::make_shared<TextComponent>(mWindow, getCleanFileName(mSearchParams.game->getName()), Font::get(FONT_SIZE_LARGE), 0x777777FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mHeader, Eigen::Vector2i(0, 0), false, true);

	// ScraperSearchComponent
	mSearch = std::make_shared<ScraperSearchComponent>(window, ScraperSearchComponent::NEVER_AUTO_ACCEPT);
	mGrid.setEntry(mSearch, Eigen::Vector2i(0, 1), true);

	// buttons
	std::vector< std::shared_ptr<ButtonComponent> > buttons;

	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "INPUT", "search", [&] { 
		mSearch->openInputScreen(mSearchParams); 
		mGrid.resetCursor(); 
	}));
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "CANCEL", "cancel", [&] { delete this; }));
	mButtonGrid = makeButtonGrid(mWindow, buttons);

	mGrid.setEntry(mButtonGrid, Eigen::Vector2i(0, 2), true, false);

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

	setSize(Renderer::getScreenWidth() * 0.7f, Renderer::getScreenHeight() * 0.65f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);

	mGrid.resetCursor();
	mSearch->search(params); // start the search
}

void GuiGameScraper::onSizeChanged()
{
	mBox.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));

	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, mHeader->getFont()->getHeight() / mSize.y()); // header
	mGrid.setRowHeightPerc(2, mButtonGrid->getSize().y() / mSize.y()); // buttons
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