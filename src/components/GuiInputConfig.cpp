#include "GuiInputConfig.h"
#include "GuiGameList.h"
#include <iostream>
#include <fstream>
#include "../Log.h"

extern bool DEBUG; //defined in main.cpp

std::string GuiInputConfig::sInputs[] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "START", "SELECT", "PAGEUP", "PAGEDOWN" }; //must be same order as InputManager::InputButton enum; only add to the end to preserve backwards compatibility
int GuiInputConfig::sInputCount = 10;

GuiInputConfig::GuiInputConfig(Window* window) : Gui(window)
{
}

GuiInputConfig::~GuiInputConfig()
{
}

void GuiInputConfig::render()
{
	Renderer::drawCenteredText("IN DEVELOPMENT", 0, 2, 0x000000FF, Renderer::getDefaultFont(Renderer::MEDIUM));
}

void GuiInputConfig::input(InputConfig* config, Input input)
{
}
