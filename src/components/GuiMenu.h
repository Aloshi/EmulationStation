#ifndef _GUIMENU_H_
#define _GUIMENU_H_

#include "../GuiComponent.h"
#include "GuiList.h"

class GuiGameList;

//This is a very simple menu that is opened by pressing the Menu key.
class GuiMenu : public GuiComponent
{
public:
	GuiMenu(GuiGameList* parent);
	~GuiMenu();

	void onInput(InputManager::InputButton button, bool keyDown);
	void onRender();

private:
	GuiGameList* mParent;
	GuiList<std::string>* mList;

	void populateList();
	void executeCommand(std::string command);

	bool mSkippedMenuClose;
};

#endif
