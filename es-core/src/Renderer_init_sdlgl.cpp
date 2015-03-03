#include "Renderer.h"
#include <iostream>
#include "platform.h"
#include GLHEADER
#include "resources/Font.h"
#include <SDL.h>
#include "Log.h"
#include "ImageIO.h"
#include "../data/Resources.h"
#include "Settings.h"

#ifdef USE_OPENGL_ES
	#define glOrtho glOrthof
#endif

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

		// multisample anti-aliasing
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

#ifdef USE_OPENGL_ES
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
#endif

		SDL_DisplayMode dispMode;
		SDL_GetDesktopDisplayMode(0, &dispMode);
		if(display_width == 0)
			display_width = dispMode.w;
		if(display_height == 0)
			display_height = dispMode.h;

		sdlWindow = SDL_CreateWindow("EmulationStation", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			display_width, display_height, 
			SDL_WINDOW_OPENGL | (Settings::getInstance()->getBool("Windowed") ? 0 : SDL_WINDOW_FULLSCREEN));

		if(sdlWindow == NULL)
		{
			LOG(LogError) << "Error creating SDL window!\n\t" << SDL_GetError();
			return false;
		}

		LOG(LogInfo) << "Created window successfully.";

		//set an icon for the window
		size_t width = 0;
		size_t height = 0;
		std::vector<unsigned char> rawData = ImageIO::loadFromMemoryRGBA32(window_icon_256_png_data, window_icon_256_png_size, width, height);
		if (!rawData.empty())
		{
			ImageIO::flipPixelsVert(rawData.data(), width, height);

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

		// vsync
		if(Settings::getInstance()->getBool("VSync"))
		{
			// SDL_GL_SetSwapInterval(0) for immediate updates (no vsync, default), 
			// 1 for updates synchronized with the vertical retrace, 
			// or -1 for late swap tearing.
			// SDL_GL_SetSwapInterval returns 0 on success, -1 on error.
			// if vsync is requested, try late swap tearing; if that doesn't work, try normal vsync
			// if that doesn't work, report an error
			if(SDL_GL_SetSwapInterval(-1) != 0 && SDL_GL_SetSwapInterval(1) != 0)
				LOG(LogWarning) << "Tried to enable vsync, but failed! (" << SDL_GetError() << ")";
		}

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

		return true;
	}

	void deinit()
	{
		destroySurface();
	}
};
