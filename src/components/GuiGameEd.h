#pragma once

#include "../GuiComponent.h"
#include "ComponentListComponent.h"
#include "../MetaData.h"
#include "TextComponent.h"
#include "../GameData.h"
#include "NinePatchComponent.h"
#include "ButtonComponent.h"

class GuiGameEd : public GuiComponent
{
public:
	GuiGameEd(Window* window, GameData* game, const std::vector<MetaDataDecl>& mdd);
	virtual ~GuiGameEd();

private:
	void saveGame();
	void deleteGame();

	void populateList(const std::vector<MetaDataDecl>& mdd);

	NinePatchComponent mBox;

	ComponentListComponent mList;

	TextComponent mPathDisp;

	std::vector<TextComponent*> mLabels;
	std::vector<GuiComponent*> mEditors;

	GameData* mGame;

	ButtonComponent mDeleteButton;
	ButtonComponent mFetchButton;
	ButtonComponent mSaveButton;
};
