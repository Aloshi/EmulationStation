#include "Renderer.h"
#include <iostream>
#include "platform.h"

#ifdef _WINDOWS_
	#include <Windows.h>
#endif

#include GLHEADER

#include "Font.h"
#include <SDL.h>
#include "InputManager.h"
#include "Log.h"
#include "ImageIO.h"
#include "../data/Resources.h"
#include "EmulationStation.h"
#include "Settings.h"

namespace Renderer
{
	static bool initialCursorState;

	unsigned int display_width = 0;
	unsigned int display_height = 0;

	unsigned int getScreenWidth() { return display_width; }
	unsigned int getScreenHeight() { return display_height; }

	SDL_Surface* sdlScreen = NULL;

	bool createSurface() //unsigned int display_width, unsigned int display_height)
	{
		LOG(LogInfo) << "Creating surface...";

		if(SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			LOG(LogError) << "Error initializing SDL!\n	" << SDL_GetError();
			return false;
		}

		//ATM it is best to just leave the window icon alone on windows.
		//When compiled as a Windows application, ES at least has an icon in the taskbar
		//The method below looks pretty shite as alpha isn't taken into account...
#ifndef WIN32
		//try loading PNG from memory
		size_t width = 0;
		size_t height = 0;
		std::vector<unsigned char> rawData = ImageIO::loadFromMemoryRGBA32(ES_logo_32_png_data, ES_logo_32_png_size, width, height);
		if (!rawData.empty()) {
			//SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Uint32 rmask = 0xff000000; Uint32 gmask = 0x0000ff00; Uint32 bmask = 0x00ff0000; Uint32 amask = 0x000000ff;
#else
			Uint32 rmask = 0x000000ff; Uint32 gmask = 0x00ff0000; Uint32 bmask = 0x0000ff00; Uint32 amask = 0xff000000;
#endif
			//try creating SDL surface from logo data
			SDL_Surface * logoSurface = SDL_CreateRGBSurfaceFrom((void *)rawData.data(), width, height, 32, width*4, rmask, gmask, bmask, amask);
			if (logoSurface != nullptr) {
				//change window icon. this sucks atm, but there's nothing better we can do. SDL 1.3 or 2.0 should sort this out...
				SDL_WM_SetIcon(logoSurface, nullptr);
			}
		}
#endif

		SDL_WM_SetCaption("EmulationStation", "EmulationStation");

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		//SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); //vsync
		sdlScreen = SDL_SetVideoMode(display_width, display_height, 16, SDL_OPENGL | (Settings::getInstance()->getBool("WINDOWED") ? 0 : SDL_FULLSCREEN));

		if(sdlScreen == NULL)
		{
			LOG(LogError) << "Error creating SDL video surface!";
			return false;
		}

		//usually display width/height are not specified, i.e. zero, which SDL automatically takes as "native resolution"
		//so, since other things rely on the size of the screen (damn currently unnormalized coordinate system), we set it here
		//even though the system was already initialized
		display_width = sdlScreen->w;
		display_height = sdlScreen->h;

		LOG(LogInfo) << "Created surface successfully.";

		//hide mouse cursor
		initialCursorState = SDL_ShowCursor(0) == 1;

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

		//show mouse cursor
		SDL_ShowCursor(initialCursorState);

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

		glViewport(0, 0, display_width, display_height);

		glMatrixMode(GL_PROJECTION);
		glOrtho(0, display_width, display_height, 0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		onInit();

		return true;
	}

	void deinit()
	{
		onDeinit();

		destroySurface();
	}
};
