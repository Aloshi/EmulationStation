#pragma once
#ifndef ES_APP_GUIS_GUI_FAST_SELECT_H
#define ES_APP_GUIS_GUI_FAST_SELECT_H

#include "GuiComponent.h"
#include "views/gamelist/IGameListView.h"

#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"

class GuiFastSelect : public GuiComponent
{
public:
	GuiFastSelect(Window* window, IGameListView* gamelist);

	bool input(InputConfig* config, Input input);
	void update(int deltaTime);

private:
	void setScrollDir(int dir);
	void scroll();
	void updateGameListCursor();
	void updateGameListSort();
	void updateSortText();

	int mSortId;
	int mLetterId;

	int mScrollDir;
	int mScrollAccumulator;

	NinePatchComponent mBackground;
	TextComponent mSortText;
	TextComponent mLetterText;

	IGameListView* mGameList;
};

#endif // ES_APP_GUIS_GUI_FAST_SELECT_H
