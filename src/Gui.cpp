#include "Gui.h"
#include "Window.h"

Gui::Gui(Window* window) : mWindow(window)
{
}

Gui::~Gui()
{
	mWindow->removeGui(this);
}

void Gui::setOffset(int x, int y) { mOffsetX = x; mOffsetY = y; }
void Gui::setOffsetX(int x) { mOffsetX = x; }
void Gui::setOffsetY(int y) { mOffsetY = y; }
int Gui::getOffsetX() { return mOffsetX; }
int Gui::getOffsetY() { return mOffsetY; }
