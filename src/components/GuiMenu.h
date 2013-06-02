#ifndef _GUIMENU_H_
#define _GUIMENU_H_

#include "../GuiComponent.h"
#include "TextListComponent.h"

class GuiGameList;

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window, GuiGameList* parent);
	virtual ~GuiMenu();

	bool input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

private:
	GuiGameList* mParent;
	TextListComponent<std::string>* mList;

	void populateList();
	void executeCommand(std::string command);
};

#endif
