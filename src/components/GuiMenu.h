#ifndef _GUIMENU_H_
#define _GUIMENU_H_

#include "../Gui.h"
#include "GuiList.h"

class GuiGameList;

class GuiMenu : public Gui
{
public:
	GuiMenu(Window* window, GuiGameList* parent);
	~GuiMenu();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

private:
	GuiGameList* mParent;
	GuiList<std::string>* mList;

	void populateList();
	void executeCommand(std::string command);
};

#endif
