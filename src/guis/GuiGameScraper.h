#pragma once

#include "../GuiComponent.h"
#include "../components/ScraperSearchComponent.h"
#include "../components/NinePatchComponent.h"

class GuiGameScraper : public GuiComponent
{
public:
	GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(MetaDataList)> doneFunc, std::function<void()> skipFunc = nullptr);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	int mSearchCountdown; // haaack

	ComponentGrid mGrid;
	NinePatchComponent mBox;

	std::shared_ptr<ScraperSearchComponent> mSearch;

	ScraperSearchParams mSearchParams;

	std::function<void(MetaDataList)> mDoneFunc;
	std::function<void()> mSkipFunc;
};
