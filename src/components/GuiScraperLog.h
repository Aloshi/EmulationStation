#pragma once

#include "../GuiComponent.h"
#include "NinePatchComponent.h"
#include <queue>
#include "../scrapers/Scraper.h"
#include <boost/circular_buffer.hpp>
#include "TextComponent.h"

//A "terminal" of sorts for scraping.
//Doesn't accept input, but renders log-style messages and handles the callback chain for multi-game scraping.
class GuiScraperLog : public GuiComponent
{
public:
	GuiScraperLog(Window* window, const std::queue<ScraperSearchParams>& params, bool manualMode);
	~GuiScraperLog();

	void start();

	void render(const Eigen::Affine3f& parentTrans) override;
	bool input(InputConfig* config, Input input) override;

private:
	void updateStatus();
	void writeLine(const std::string& line, unsigned int color);

	void resultFetched(ScraperSearchParams params, MetaDataList mdl);
	void resultResolved(ScraperSearchParams params, MetaDataList mdl);
	void resultEmpty(ScraperSearchParams params);

	void next();
	void done();

	bool mManualMode;

	NinePatchComponent mBox;

	std::queue<ScraperSearchParams> mSearches;
	
	TextComponent mStatus;
	boost::circular_buffer< std::shared_ptr<TextComponent> > mTextLines;

	unsigned int mSuccessCount;
	unsigned int mSkippedCount;
};
