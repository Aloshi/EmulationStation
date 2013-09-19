#pragma once

#include "../GuiComponent.h"
#include "ComponentListComponent.h"
#include "../MetaData.h"
#include "TextComponent.h"
#include "../GameData.h"
#include "NinePatchComponent.h"
#include "ButtonComponent.h"
#include <functional>
#include "../scrapers/Scraper.h"

class GuiMetaDataEd : public GuiComponent
{
public:
	GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, ScraperSearchParams params, 
		const std::string& header, std::function<void()> savedCallback, std::function<void()> deleteFunc);
	virtual ~GuiMetaDataEd();

private:
	void save();
	void fetch();
	void fetchDone(MetaDataList result);

	void populateList(const std::vector<MetaDataDecl>& mdd);

	ScraperSearchParams mScraperParams;

	NinePatchComponent mBox;

	ComponentListComponent mList;

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
