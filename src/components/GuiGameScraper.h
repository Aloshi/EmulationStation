#pragma once

#include "../GuiComponent.h"
#include "../scrapers/Scraper.h"
#include "ComponentListComponent.h"
#include "TextComponent.h"
#include "ScrollableContainer.h"
#include "TextEditComponent.h"
#include "NinePatchComponent.h"
#include "../Settings.h"

class GuiGameScraper : public GuiComponent
{
public:
	GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(MetaDataList)> doneFunc, std::function<void()> skipFunc = NULL);

	bool input(InputConfig* config, Input input) override;

	void search();
private:
	int getSelectedIndex();
	void onSearchDone(std::vector<MetaDataList> results);

	ComponentListComponent mList;
	NinePatchComponent mBox;

	TextComponent mHeader;

	TextComponent mResultName;
	ScrollableContainer mResultInfo;
	TextComponent mResultDesc;

	TextComponent mSearchLabel;
	TextEditComponent mSearchText;

	std::vector<TextComponent> mResultNames;

	ScraperSearchParams mSearchParams;

	std::vector<MetaDataList> mScraperResults;

	std::function<void(MetaDataList)> mDoneFunc;
	std::function<void()> mSkipFunc;
};
