#include "GuiScraperLog.h"
#include "../Settings.h"
#include "GuiGameScraper.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../XMLReader.h"

GuiScraperLog::GuiScraperLog(Window* window, const std::queue<ScraperSearchParams>& searches, bool manualMode) : GuiComponent(window), 
	mManualMode(manualMode), 
	mBox(window, ":/frame.png"),
	mSearches(searches),
	mStatus(window), 
	mTextLines(40), 
	mSuccessCount(0), mSkippedCount(0)
{
	addChild(&mBox);
	addChild(&mStatus);

	setSize(Renderer::getScreenWidth() * 0.8f, (float)Renderer::getScreenHeight());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, 0);

	mStatus.setColor(0x000000FF);
	mStatus.setSize(mSize.x(), (float)mStatus.getFont()->getHeight());
	mStatus.setCentered(true);
	updateStatus();

	mBox.setEdgeColor(0x111111FF);
	mBox.setCenterColor(0xDFDFDFFF);
	mBox.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(8, 0));

	mWindow->setAllowSleep(false);
}

GuiScraperLog::~GuiScraperLog()
{
	mWindow->setAllowSleep(true);
}

void GuiScraperLog::start()
{
	next();
}

void GuiScraperLog::next()
{
	if(mSearches.empty())
	{
		done();
		return;
	}

	ScraperSearchParams search = mSearches.front();
	mSearches.pop();

	writeLine(getCleanFileName(search.game->getPath()), 0x0000FFFF);

	if(mManualMode)
	{
		GuiGameScraper* ggs = new GuiGameScraper(mWindow, search, 
			[this, search] (MetaDataList result) { resultFetched(search, result); }, 
			[this, search] { resultEmpty(search); });

		mWindow->pushGui(ggs);
		ggs->search();
	}else{
		std::shared_ptr<Scraper> scraper = Settings::getInstance()->getScraper();
		scraper->getResultsAsync(search, mWindow, [this, search] (std::vector<MetaDataList> mdls) { 
			if(mdls.empty())
				resultEmpty(search);
			else
				resultFetched(search, mdls[0]);
		});
	}

	updateStatus();
}

void GuiScraperLog::resultFetched(ScraperSearchParams params, MetaDataList mdl)
{
	writeLine("   -> \"" + mdl.get("name") + "\"", 0x0000FFFF);
	writeLine("   Downloading images...", 0x0000FFFF);
	resolveMetaDataAssetsAsync(mWindow, params, mdl, [this, params] (MetaDataList meta) { resultResolved(params, meta); });

	//-> resultResolved
}

void GuiScraperLog::resultResolved(ScraperSearchParams params, MetaDataList mdl)
{
	//apply new metadata
	params.game->metadata = mdl;

	writeLine("   Success!", 0x00FF00FF);

	//write changes to gamelist.xml
	updateGamelist(params.system);

	mSuccessCount++;

	next();
}

void GuiScraperLog::resultEmpty(ScraperSearchParams search)
{
	if(mManualMode)
		writeLine("   SKIPPING", 0xFF0000FF);
	else
		writeLine("   NO RESULTS, skipping", 0xFF0000FF);

	LOG(LogInfo) << "Scraper skipping [" << getCleanFileName(search.game->getPath()) << "]";

	mSkippedCount++;

	next();
}

void GuiScraperLog::done()
{
	writeLine("===================================", 0x000000FF);
	
	std::stringstream ss;
	ss << mSuccessCount << " successful, " << mSkippedCount << " skipped";
	writeLine(ss.str(), 0x000000FF);

	writeLine("DONE!! Press anything to continue.", 0x00FF00FF);
	writeLine("===================================", 0x000000FF);

	//done with everything!
}

void GuiScraperLog::writeLine(const std::string& line, unsigned int color)
{
	std::shared_ptr<TextComponent> cmp(new TextComponent(mWindow));
	cmp->setText(line);
	cmp->setColor(color);
	cmp->setSize(mSize.x(), (float)cmp->getFont()->getHeight());

	mTextLines.push_back(cmp);
}

void GuiScraperLog::render(const Eigen::Affine3f& parentTrans)
{
	renderChildren(parentTrans * getTransform());
	
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	//draw messages
	float fontHeight = (float)Font::get(FONT_SIZE_MEDIUM)->getHeight();
	trans = trans.translate(Eigen::Vector3f(0, mSize.y() - fontHeight, 0));
	
	for(auto it = mTextLines.rbegin(); it != mTextLines.rend(); it++)
	{
		(*it)->render(trans);
		trans = trans.translate(Eigen::Vector3f(0, -fontHeight, 0));

		if(trans.translation().y() < fontHeight)
			break;
	}
}

bool GuiScraperLog::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		//we're done
		if(mSearches.empty())
		{
			delete this;
			return true;
		}
	}

	return GuiComponent::input(config, input);
}

void GuiScraperLog::updateStatus()
{
	std::stringstream ss;

	ss << mSearches.size() << " games remaining";

	mStatus.setText(ss.str());
}
