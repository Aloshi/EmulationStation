#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Renderer.h"
#include "components/GuiGameList.h"
#include "SystemData.h"
#include <boost/filesystem.hpp>
#include "components/GuiInputConfig.h"

int main()
{
	bool running = true;

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

	if(!boost::filesystem::exists(SystemData::getConfigPath()))
	{
		std::cerr << "A system config file in $HOME/.es_systems.cfg was not found. An example will be created.\n";
		SystemData::writeExampleConfig();
		std::cerr << "Set it up, then re-run EmulationStation.\n";
		running = false;
	}else{
		SystemData::loadConfig();

		if(SystemData::sSystemVector.size() == 0)
		{
			std::cerr << "A system config file in $HOME/.es_systems.cfg was found, but contained no systems.\n";
			std::cerr << "You should probably go read that, or delete it.\n";
			running = false;
		}else{
			if(boost::filesystem::exists(InputManager::getConfigPath()))
			{
				InputManager::loadConfig();
				new GuiGameList();
			}else{
				if(SDL_NumJoysticks() > 0)
				{
					new GuiInputConfig();
				}else{
					std::cout << "Note - it looks like you have no joysticks connected.\n";
					new GuiGameList();
				}
			}
		}
	}


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
