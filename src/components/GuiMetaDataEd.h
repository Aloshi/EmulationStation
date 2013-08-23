#pragma once

#include "../GuiComponent.h"
#include "ComponentListComponent.h"
#include "../MetaData.h"
#include "TextComponent.h"
#include "../GameData.h"
#include "NinePatchComponent.h"
#include "ButtonComponent.h"
#include <functional>

class GuiMetaDataEd : public GuiComponent
{
public:
	GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd,
		const std::string& header, std::function<void()> savedCallback, std::function<void()> deleteFunc);
	virtual ~GuiMetaDataEd();

private:
	void save();

	void populateList(const std::vector<MetaDataDecl>& mdd);

	NinePatchComponent mBox;

	ComponentListComponent mList;

	TextComponent mHeader;

	std::vector<TextComponent*> mLabels;
	std::vector<GuiComponent*> mEditors;

	MetaDataList* mMetaData;
	std::function<void()> mSavedCallback;
	std::function<void()> mDeleteFunc;

	ButtonComponent mDeleteButton;
	ButtonComponent mFetchButton;
	ButtonComponent mSaveButton;
};
