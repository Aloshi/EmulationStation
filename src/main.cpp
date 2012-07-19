#include <iostream>
#include <SDL/SDL.h>
#include "Renderer.h"
#include "components/GuiTitleScreen.h"

int main()
{
	//if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "Error - could not initialize SDL!\n";
		return 1;
	}

	Renderer::screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
	if(Renderer::screen == NULL)
	{
		std::cerr << "Error - could not set video mode!\n";
		return 1;
	}


	GuiTitleScreen* testGui = new GuiTitleScreen();

	bool running = true;
	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					running = false;
					break;
				case SDL_QUIT:
					running = false;
					break;
			}
		}

		Renderer::render();
		SDL_Flip(Renderer::screen);
	}

	delete testGui;

	SDL_Quit();
	return 0;
}
