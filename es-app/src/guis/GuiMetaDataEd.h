#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "MetaData.h"
#include "scrapers/Scraper.h"

#include <functional>

class GuiMetaDataEd : public GuiComponent
{
public:
	GuiMetaDataEd(Window* window, const FileData& file, 
		const std::function<void()>& savedCallback, const std::function<void()>& deleteFunc);

	bool input(InputConfig* config, Input input) override;
	void onSizeChanged() override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void save();
	void fetch();
	void fetchDone(const ScraperSearchResult& result);
	void close(bool closeAllWindows);

	NinePatchComponent mBackground;
	ComponentGrid mGrid;
	
	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextComponent> mSubtitle;
	std::shared_ptr<ComponentGrid> mHeaderGrid;
	std::shared_ptr<ComponentList> mList;
	std::shared_ptr<ComponentGrid> mButtons;

	FileData mFile;
	MetaDataMap mMetaData;
	const std::vector<MetaDataDecl>& mMetaDataDecl;

	ScraperSearchParams mScraperParams;

	std::vector< std::shared_ptr<GuiComponent> > mEditors;
	
	std::function<void()> mSavedCallback;
	std::function<void()> mDeleteFunc;
};
