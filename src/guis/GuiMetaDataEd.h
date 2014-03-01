#pragma once

#include "../GuiComponent.h"
#include "../components/ComponentGrid.h"
#include "../MetaData.h"
#include "../components/TextComponent.h"
#include "../components/NinePatchComponent.h"
#include "../components/ButtonComponent.h"
#include "../scrapers/Scraper.h"

#include <functional>

class GuiMetaDataEd : public GuiComponent
{
public:
	GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams params, 
		const std::string& header, std::function<void()> savedCallback, std::function<void()> deleteFunc);
	virtual ~GuiMetaDataEd();

	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void save();
	void fetch();
	void fetchDone(MetaDataList result);

	void populateList(const std::vector<MetaDataDecl>& mdd);

	ScraperSearchParams mScraperParams;

	NinePatchComponent mBox;

	ComponentGrid mList;

	TextComponent mHeader;

	std::vector<TextComponent*> mLabels;
	std::vector<GuiComponent*> mEditors;

	std::vector<MetaDataDecl> mMetaDataDecl;
	MetaDataList* mMetaData;
	std::function<void()> mSavedCallback;
	std::function<void()> mDeleteFunc;

	ButtonComponent mDeleteButton;
	ButtonComponent mFetchButton;
	ButtonComponent mSaveButton;
};
