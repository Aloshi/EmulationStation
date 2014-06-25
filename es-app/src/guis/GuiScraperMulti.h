#pragma once

#include "GuiComponent.h"
#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include "scrapers/Scraper.h"

#include <queue>

class ScraperSearchComponent;
class TextComponent;

class GuiScraperMulti : public GuiComponent
{
public:
	GuiScraperMulti(Window* window, const std::queue<ScraperSearchParams>& searches, bool approveResults);
	virtual ~GuiScraperMulti();

	void onSizeChanged() override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void acceptResult(const ScraperSearchResult& result);
	void skip();
	void doNextSearch();
	
	void finish();

	unsigned int mTotalGames;
	unsigned int mCurrentGame;
	unsigned int mTotalSuccessful;
	unsigned int mTotalSkipped;
	std::queue<ScraperSearchParams> mSearchQueue;

	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextComponent> mSystem;
	std::shared_ptr<TextComponent> mSubtitle;
	std::shared_ptr<ScraperSearchComponent> mSearchComp;
	std::shared_ptr<ComponentGrid> mButtonGrid;
};
