#pragma once

#include "../GuiComponent.h"
#include "../scrapers/Scraper.h"
#include "../components/ComponentGrid.h"
#include <functional>

#define MAX_SCRAPER_RESULTS 5

class ComponentList;
class TextEditComponent;
class ImageComponent;
class ScrollableContainer;
class HttpReq;

class ScraperSearchComponent : public GuiComponent
{
public:
	enum SearchType
	{
		ALWAYS_ACCEPT_FIRST_RESULT,
		ALWAYS_ACCEPT_MATCHING_CRC,
		NEVER_AUTO_ACCEPT
	};

	ScraperSearchComponent(Window* window, SearchType searchType = NEVER_AUTO_ACCEPT);

	void setSearchParams(const ScraperSearchParams& params);
	inline void setAcceptCallback(const std::function<void(MetaDataList*)>& acceptCallback) { mAcceptCallback = acceptCallback; }

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	void onSizeChanged() override;	
	void onFocusGained() override;
	void onFocusLost() override;

private:
	void updateViewStyle();
	void updateThumbnail();
	void updateInfoPane();

	void search();
	void onSearchReceived(std::vector<MetaDataList> results);

	int getSelectedIndex();

	void returnResult(MetaDataList* result);

	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mResultName;
	std::shared_ptr<ScrollableContainer> mDescContainer;
	std::shared_ptr<TextComponent> mResultDesc;
	std::shared_ptr<ImageComponent> mResultThumbnail;
	std::shared_ptr<ComponentList> mResultList;

	SearchType mSearchType;
	ScraperSearchParams mSearchParams;
	std::function<void(MetaDataList*)> mAcceptCallback;

	std::vector<MetaDataList> mScraperResults;
	std::unique_ptr<HttpReq> mThumbnailReq;
};
