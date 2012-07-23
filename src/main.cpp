#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Renderer.h"
#include "components/GuiGameList.h"
#include "SystemData.h"

#include "components/GuiInputConfig.h"

int main()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
	{
		std::cerr << "Error - could not initialize SDL!\n";
		std::cerr << "	" << SDL_GetError() << "\n";
		std::cerr << "\nAre you in the 'video' and 'input' groups?\n";
		return 1;
	}
	if(TTF_Init() != 0)
	{
		std::cerr << "Error - could not initialize SDL_ttf!\n";
		std::cerr << "	" << TTF_GetError() << "\n";
		return 1;
	}

	Renderer::screen = SDL_SetVideoMode(Renderer::getScreenWidth(), Renderer::getScreenHeight(), 16, SDL_SWSURFACE);
	if(Renderer::screen == NULL)
	{
		std::cerr << "Error - could not set video mode!\n";
		std::cerr << "	" << SDL_GetError() << "\n";
		return 1;
	}

	SDL_ShowCursor(false);
	SDL_EnableKeyRepeat(500, 100);
	SDL_JoystickEventState(SDL_ENABLE);

	//GuiTitleScreen* testGui = new GuiTitleScreen();

	SystemData::loadConfig("./systems.cfg");

	//InputManager::loadConfig("./input.cfg");

	//GuiGameList* testGui = new GuiGameList();
	GuiInputConfig* testGui = new GuiInputConfig();

	bool running = true;
	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_JOYAXISMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
				case SDL_KEYDOWN:
					InputManager::processEvent(&event);
					break;
				case SDL_KEYUP:
					InputManager::processEvent(&event);
					break;

				case SDL_QUIT:
					running = false;
					break;
			}
		}

		Renderer::render();
		SDL_Flip(Renderer::screen);
	}

	Renderer::deleteAll();
	SystemData::deleteSystems();

	std::cout << "EmulationStation cleanly shutting down...\n";

	TTF_Quit();
	SDL_Quit();
	return 0;
}
