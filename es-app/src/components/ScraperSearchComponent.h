#pragma once

#include "GuiComponent.h"
#include "scrapers/Scraper.h"
#include "components/ComponentGrid.h"
#include "components/BusyComponent.h"
#include <functional>

class ComponentList;
class ImageComponent;
class RatingComponent;
class TextComponent;
class DateTimeComponent;
class ScrollableContainer;
class HttpReq;
class AnimatedImageComponent;

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

	void search(const ScraperSearchParams& params);
	void openInputScreen(ScraperSearchParams& from);
	void stop();
	inline SearchType getSearchType() const { return mSearchType; }

	// Metadata assets will be resolved before calling the accept callback (e.g. result.mdl's "image" is automatically downloaded and properly set).
	inline void setAcceptCallback(const std::function<void(const ScraperSearchResult&)>& acceptCallback) { mAcceptCallback = acceptCallback; }
	inline void setSkipCallback(const std::function<void()>& skipCallback) { mSkipCallback = skipCallback; };
	inline void setCancelCallback(const std::function<void()>& cancelCallback) { mCancelCallback = cancelCallback; }

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	void onSizeChanged() override;	
	void onFocusGained() override;
	void onFocusLost() override;

private:
	void updateViewStyle();
	void updateThumbnail();
	void updateFanart();
	void updateInfoPane();

	void resizeMetadata();

	void onSearchError(const std::string& error);
	void onSearchDone(const std::vector<ScraperSearchResult>& results);

	int getSelectedIndex();

	// resolve any metadata assets that need to be downloaded and return
	void returnResult(ScraperSearchResult result);

	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mResultName;
	std::shared_ptr<ScrollableContainer> mDescContainer;
	std::shared_ptr<TextComponent> mResultDesc;
	std::shared_ptr<ImageComponent> mResultThumbnail;
	std::shared_ptr<ImageComponent> mResultFanart;
	std::shared_ptr<ComponentList> mResultList;


	// label-component pair
	struct MetaDataPair
	{
		std::shared_ptr<TextComponent> first;
		std::shared_ptr<GuiComponent> second;
		bool resize;

		MetaDataPair(const std::shared_ptr<TextComponent>& f, const std::shared_ptr<GuiComponent>& s, bool r = true) : first(f), second(s), resize(r) {};
	};
	
	std::vector<MetaDataPair> mMD_Pairs;

	SearchType mSearchType;
	ScraperSearchParams mLastSearch;
	std::function<void(const ScraperSearchResult&)> mAcceptCallback;
	std::function<void()> mSkipCallback;
	std::function<void()> mCancelCallback;
	bool mBlockAccept;

	std::unique_ptr<ScraperSearchHandle> mSearchHandle;
	std::unique_ptr<MDResolveHandle> mMDResolveHandle;
	std::vector<ScraperSearchResult> mScraperResults;
	std::unique_ptr<HttpReq> mThumbnailReq;
	std::unique_ptr<HttpReq> mFanartReq;

	BusyComponent mBusyAnim;
};
