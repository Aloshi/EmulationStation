#pragma once

#include "../GuiComponent.h"
#include "../scrapers/Scraper.h"
#include "ComponentListComponent.h"
#include "TextComponent.h"
#include "ScrollableContainer.h"
#include "TextEditComponent.h"
#include "NinePatchComponent.h"
#include "../Settings.h"
#include "../HttpReq.h"
#include "ImageComponent.h"

#define MAX_SCRAPER_RESULTS 5

class GuiGameScraper : public GuiComponent
{
public:
	GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(MetaDataList)> doneFunc, std::function<void()> skipFunc = nullptr);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;

	void search();
private:
	int getSelectedIndex();
	void onSearchDone(std::vector<MetaDataList> results);
	void updateInfoPane();
	void updateThumbnail();

	ComponentListComponent mList;
	NinePatchComponent mBox;

	TextComponent mHeader;

	TextComponent mResultName;
	ScrollableContainer mResultInfo;
	TextComponent mResultDesc;
	ImageComponent mResultThumbnail;

	TextComponent mSearchLabel;
	TextEditComponent mSearchText;

	std::vector<TextComponent> mResultNames;

	ScraperSearchParams mSearchParams;

	std::vector<MetaDataList> mScraperResults;

	std::function<void(MetaDataList)> mDoneFunc;
	std::function<void()> mSkipFunc;

	std::unique_ptr<HttpReq> mThumbnailReq;
};
