#include "Gui.h"
#include "Window.h"

Gui::Gui(Window* window) : mWindow(window)
{
}

Gui::~Gui()
{
	mWindow->removeGui(this);
}
