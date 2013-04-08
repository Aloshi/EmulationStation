#ifndef _GUI_H_
#define _GUI_H_

#include "InputConfig.h"

class Window;

class Gui
{
public:
	Gui(Window* window);
	virtual ~Gui();

	virtual void input(InputConfig* config, Input input) = 0;
	virtual void update(int deltaTime) = 0;
	virtual void render() = 0;
protected:
	Window* mWindow;
};

#endif