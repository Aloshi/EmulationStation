#include "Renderer.h"
#include <iostream>
#include "platform.h"
#include GLHEADER
#include "Font.h"
#include <SDL.h>
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

	SDL_Window* sdlWindow = NULL;
	SDL_GLContext sdlContext = NULL;

	bool createSurface()
	{
		LOG(LogInfo) << "Creating surface...";

		if(SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			LOG(LogError) << "Error initializing SDL!\n	" << SDL_GetError();
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		//SDL_GL_SetSwapInterval(1); //0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for late swap tearing

		SDL_DisplayMode dispMode;
		SDL_GetDisplayMode(0, 0, &dispMode);
		if(display_width == 0)
			display_width = dispMode.w;
		if(display_height == 0)
			display_height = dispMode.h;

		sdlWindow = SDL_CreateWindow("EmulationStation", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			display_width, display_height, 
			SDL_WINDOW_OPENGL | (Settings::getInstance()->getBool("WINDOWED") ? 0 : SDL_WINDOW_FULLSCREEN));

		if(sdlWindow == NULL)
		{
			LOG(LogError) << "Error creating SDL window!";
			return false;
		}

		LOG(LogInfo) << "Created window successfully.";

		//set an icon for the window
		size_t width = 0;
		size_t height = 0;
		std::vector<unsigned char> rawData = ImageIO::loadFromMemoryRGBA32(ES_logo_32_png_data, ES_logo_32_png_size, width, height);
		if (!rawData.empty())
		{
			//SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
						Uint32 rmask = 0xff000000; Uint32 gmask = 0x00ff0000; Uint32 bmask = 0x0000ff00; Uint32 amask = 0x000000ff;
			#else
						Uint32 rmask = 0x000000ff; Uint32 gmask = 0x0000ff00; Uint32 bmask = 0x00ff0000; Uint32 amask = 0xff000000;
			#endif
			//try creating SDL surface from logo data
			SDL_Surface * logoSurface = SDL_CreateRGBSurfaceFrom((void *)rawData.data(), width, height, 32, width * 4, rmask, gmask, bmask, amask);
			if (logoSurface != NULL)
			{
				SDL_SetWindowIcon(sdlWindow, logoSurface);
				SDL_FreeSurface(logoSurface);
			}
		}

		sdlContext = SDL_GL_CreateContext(sdlWindow);

		//usually display width/height are not specified, i.e. zero, which SDL automatically takes as "native resolution"
		//so, since other things rely on the size of the screen (damn currently unnormalized coordinate system), we set it here
		//even though the system was already initialized - this makes sure it gets reinitialized to the original resolution when we return from a game
		//display_width = sdlWindow->w;
		//display_height = sdlWindow->h;

		//hide mouse cursor
		initialCursorState = SDL_ShowCursor(0) == 1;

		return true;
	}

	void swapBuffers()
	{
		SDL_GL_SwapWindow(sdlWindow);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void destroySurface()
	{
		SDL_GL_DeleteContext(sdlContext);
		sdlContext = NULL;

		SDL_DestroyWindow(sdlWindow);
		sdlWindow = NULL;

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
