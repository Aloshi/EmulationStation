#include "Renderer.h"
#include <iostream>
#include "platform.h"
#include GLHEADER
#include "Font.h"
#include <SDL/SDL.h>
#include "InputManager.h"

namespace Renderer
{
	unsigned int display_width = 0;
	unsigned int display_height = 0;

	unsigned int getScreenWidth() { return display_width; }
	unsigned int getScreenHeight() { return display_height; }

	SDL_Surface* sdlScreen = NULL;

	bool createSurface() //unsigned int display_width, unsigned int display_height)
	{
		std::cout << "Creating surface...";

		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
		{
			std::cerr << "Error initializing SDL!\n";
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		sdlScreen = SDL_SetVideoMode(display_width, display_height, 16, SDL_OPENGL | SDL_FULLSCREEN);

		if(sdlScreen == NULL)
		{
			std::cout << "Error creating SDL video surface!\n";
			return false;
		}

		//we need to reload input too since SDL shut down
		InputManager::loadConfig();

		//usually display width/height are not specified, i.e. zero, which SDL automatically takes as "native resolution"
		//so, since other things rely on the size of the screen (damn currently unnormalized coordinate system), we set it here
		//even though the system was already initialized
		display_width = sdlScreen->w;
		display_height = sdlScreen->h;

		std::cout << "success!\n";

		return true;
	}

	void swapBuffers()
	{
		SDL_GL_SwapBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void destroySurface()
	{
		SDL_FreeSurface(sdlScreen);
		sdlScreen = NULL;
		SDL_Quit();
	}

	bool init(int w, int h)
	{
		if(w)
			display_width = w;
		if(h)
			display_height = h;

		bool createdSurface = createSurface();

		if(!createdSurface)
			return false;

		Font::initLibrary();

		glViewport(0, 0, display_width, display_height);
		glOrtho(0, display_width, display_height, 0, -1.0, 1.0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		onInit();

		return true;
	}

	void unloadFonts(); //defined in Renderer_draw_gl.cpp
	void deinit()
	{
		onDeinit();

		unloadFonts();
		destroySurface();
	}
};
