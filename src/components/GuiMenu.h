#ifndef _GUIMENU_H_
#define _GUIMENU_H_

#include "../GuiComponent.h"
#include "GuiList.h"

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(GuiComponent* parent);
	~GuiMenu();

	void onInput(InputManager::InputButton button, bool keyDown);
	void onRender();

private:
	GuiComponent* mParent;
	GuiList<std::string>* mList;

	void populateList();

	bool mSkippedMenuClose;
};

#endif
