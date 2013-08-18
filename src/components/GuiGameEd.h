#pragma once

#include "../GuiComponent.h"
#include "ComponentListComponent.h"
#include "../MetaData.h"
#include "TextComponent.h"
#include "../GameData.h"
#include "GuiBox.h"

class GuiGameEd : public GuiComponent
{
public:
	GuiGameEd(Window* window, GameData* game, const std::vector<MetaDataDecl>& mdd);
	virtual ~GuiGameEd();

private:
	void populateList(const std::vector<MetaDataDecl>& mdd);

	GuiBox mBox;

	ComponentListComponent mList;

	TextComponent mPathDisp;

	std::vector<GuiComponent*> mGeneratedComponents;

	GameData* mGame;
};
