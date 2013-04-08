#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "Gui.h"
#include "InputManager.h"
#include <vector>

class Window
{
public:
	Window();
	~Window();

	void pushGui(Gui* gui);
	void removeGui(Gui* gui);
	Gui* peekGui();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	InputManager* getInputManager();

private:
	InputManager* mInputManager;
	std::vector<Gui*> mGuiStack;
};

#endif
