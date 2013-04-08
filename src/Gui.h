#ifndef _GUI_H_
#define _GUI_H_

#include "InputConfig.h"

class Window;

class Gui
{
public:
	Gui(Window* window);
	virtual ~Gui();

	virtual void input(InputConfig* config, Input input) { };
	virtual void update(int deltaTime) { };
	virtual void render() { };
protected:
	Window* mWindow;
};

#endif
