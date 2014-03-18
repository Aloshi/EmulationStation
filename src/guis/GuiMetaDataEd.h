#pragma once

#include "../GuiComponent.h"
#include "../components/MenuComponent.h"
#include "../MetaData.h"
#include "../scrapers/Scraper.h"

#include <functional>

class GuiMetaDataEd : public GuiComponent
{
public:
	GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams params, 
		const std::string& header, std::function<void()> savedCallback, std::function<void()> deleteFunc);
	
	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void save();
	void fetch();
	void fetchDone(const ScraperSearchResult& result);

	MenuComponent mMenu;

	ScraperSearchParams mScraperParams;

	std::vector< std::shared_ptr<GuiComponent> > mEditors;

	std::vector<MetaDataDecl> mMetaDataDecl;
	MetaDataList* mMetaData;
	std::function<void()> mSavedCallback;
	std::function<void()> mDeleteFunc;
};
