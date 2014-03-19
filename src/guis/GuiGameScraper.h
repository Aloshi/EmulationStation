#pragma once

#include "../GuiComponent.h"
#include "../components/ScraperSearchComponent.h"
#include "../components/NinePatchComponent.h"

class GuiGameScraper : public GuiComponent
{
public:
	GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(const ScraperSearchResult&)> doneFunc);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime);
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	bool mClose;
	void close();

	ComponentGrid mGrid;
	NinePatchComponent mBox;

	std::shared_ptr<ScraperSearchComponent> mSearch;

	ScraperSearchParams mSearchParams;

	std::function<void()> mCancelFunc;
};
