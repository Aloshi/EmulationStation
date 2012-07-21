#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../GuiComponent.h"
#include "GuiList.h"
#include <string>
#include "../SystemData.h"
#include "../GameData.h"

class GuiGameList : GuiComponent
{
public:
	GuiGameList();
	~GuiGameList();

	void updateList();
	void setSystemId(int id);

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);
private:
	SystemData* mSystem;
	int mSystemId;
	GuiList* mList;
};

#endif
