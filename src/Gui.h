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

	virtual void init() { };
	virtual void deinit() { };

	void setOffsetX(int x);
	void setOffsetY(int y);
	void setOffset(int x, int y);
	int getOffsetX();
	int getOffsetY();
protected:
	int mOffsetX;
	int mOffsetY;

	Window* mWindow;
};

#endif
