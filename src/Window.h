#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "GuiComponent.h"
#include "InputManager.h"
#include <vector>

class Window
{
public:
	Window();
	~Window();

	void pushGui(GuiComponent* gui);
	void removeGui(GuiComponent* gui);
	GuiComponent* peekGui();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	void init();
	void deinit();

	InputManager* getInputManager();

private:
	InputManager* mInputManager;
	std::vector<GuiComponent*> mGuiStack;
};

#endif
