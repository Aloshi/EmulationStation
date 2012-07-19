#include <iostream>
#include <SDL/SDL.h>
#include "Renderer.h"


int main()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
	{
		std::cerr << "Error - could not initialize SDL!\n";
		return 1;
	}

	

	SDL_Quit();
	return 0;
}
