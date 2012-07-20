#include "GuiTitleScreen.h"
#include "../Renderer.h"
#include <iostream>

GuiTitleScreen::GuiTitleScreen()
{
	Renderer::registerComponent(this);

	//add children here
}

void GuiTitleScreen::onRender()
{
	Renderer::drawRect(0, 0, 640, 480, 0xFFFFFF);
	SDL_Color color = {255, 0, 0};
	Renderer::drawCenteredText("EmulationStation", 5, color);
}
