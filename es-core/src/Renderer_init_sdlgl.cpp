#include "Renderer.h"

#include "resources/ResourceManager.h"
#include "ImageIO.h"
#include "Log.h"
#include "Settings.h"
#include <SDL.h>

#ifdef USE_OPENGL_ES
	#define glOrtho glOrthof
#endif

namespace Renderer
{
	static bool initialCursorState;

	unsigned int windowWidth   = 0;
	unsigned int windowHeight  = 0;
	unsigned int screenWidth   = 0;
	unsigned int screenHeight  = 0;
	unsigned int screenOffsetX = 0;
	unsigned int screenOffsetY = 0;
	unsigned int screenRotate  = 0;

	unsigned int getWindowWidth()   { return windowWidth; }
	unsigned int getWindowHeight()  { return windowHeight; }
	unsigned int getScreenWidth()   { return screenWidth; }
	unsigned int getScreenHeight()  { return screenHeight; }
	unsigned int getScreenOffsetX() { return screenOffsetX; }
	unsigned int getScreenOffsetY() { return screenOffsetY; }
	unsigned int getScreenRotate()  { return screenRotate; }

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

		//hide mouse cursor early
		initialCursorState = SDL_ShowCursor(0) == 1;

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
		windowWidth   = Settings::getInstance()->getInt("WindowWidth")   ? Settings::getInstance()->getInt("WindowWidth")   : dispMode.w;
		windowHeight  = Settings::getInstance()->getInt("WindowHeight")  ? Settings::getInstance()->getInt("WindowHeight")  : dispMode.h;
		screenWidth   = Settings::getInstance()->getInt("ScreenWidth")   ? Settings::getInstance()->getInt("ScreenWidth")   : windowWidth;
		screenHeight  = Settings::getInstance()->getInt("ScreenHeight")  ? Settings::getInstance()->getInt("ScreenHeight")  : windowHeight;
		screenOffsetX = Settings::getInstance()->getInt("ScreenOffsetX") ? Settings::getInstance()->getInt("ScreenOffsetX") : 0;
		screenOffsetY = Settings::getInstance()->getInt("ScreenOffsetY") ? Settings::getInstance()->getInt("ScreenOffsetY") : 0;
		screenRotate  = Settings::getInstance()->getInt("ScreenRotate")  ? Settings::getInstance()->getInt("ScreenRotate")  : 0;

		sdlWindow = SDL_CreateWindow("EmulationStation", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			windowWidth, windowHeight, 
			SDL_WINDOW_OPENGL | (Settings::getInstance()->getBool("Windowed") ? 0 : (Settings::getInstance()->getBool("FullscreenBorderless") ? SDL_WINDOW_BORDERLESS : SDL_WINDOW_FULLSCREEN)));

		if(sdlWindow == NULL)
		{
			LOG(LogError) << "Error creating SDL window!\n\t" << SDL_GetError();
			return false;
		}

		LOG(LogInfo) << "Created window successfully.";

		//support screen rotation
		if((screenRotate == 1) || (screenRotate == 3))
		{
			int temp;
			temp          = windowWidth;
			windowWidth   = windowHeight;
			windowHeight  = temp;
			temp          = screenWidth;
			screenWidth   = screenHeight;
			screenHeight  = temp;
			temp          = screenOffsetX;
			screenOffsetX = screenOffsetY;
			screenOffsetY = temp;
		}

		//set an icon for the window
		size_t width = 0;
		size_t height = 0;
		ResourceData resData = ResourceManager::getInstance()->getFileData(":/window_icon_256.png");
		std::vector<unsigned char> rawData = ImageIO::loadFromMemoryRGBA32(resData.ptr.get(), resData.length, width, height);
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
			SDL_Surface * logoSurface = SDL_CreateRGBSurfaceFrom((void *)rawData.data(), (int)width, (int)height, 32, (int)(width * 4), rmask, gmask, bmask, amask);
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
			// if vsync is requested, try normal vsync; if that doesn't work, try late swap tearing
			// if that doesn't work, report an error
			if(SDL_GL_SetSwapInterval(1) != 0 && SDL_GL_SetSwapInterval(-1) != 0)
				LOG(LogWarning) << "Tried to enable vsync, but failed! (" << SDL_GetError() << ")";
		}
		else
			SDL_GL_SetSwapInterval(0);

		return true;
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

	bool init()
	{
		if(!createSurface())
			return false;

		//gotta flip y since y=0 is at the bottom
		switch(screenRotate)
		{
			case 0:
			{
				glViewport(screenOffsetX, windowHeight - screenHeight - screenOffsetY, screenWidth, screenHeight);
				glMatrixMode(GL_PROJECTION);
				glOrtho(0, screenWidth, screenHeight, 0, -1.0, 1.0);
			}
			break;

			case 1:
			{
				glViewport(screenOffsetY, windowWidth - screenWidth - screenOffsetX, screenHeight, screenWidth);
				glMatrixMode(GL_PROJECTION);
				glOrtho(0, screenHeight, screenWidth, 0, -1.0, 1.0);
				glRotatef(90, 0, 0, 1);
				glTranslatef(0, screenHeight * -1.0f, 0);
			}
			break;

			case 2:
			{
				glViewport(screenOffsetX, windowHeight - screenHeight - screenOffsetY, screenWidth, screenHeight);
				glMatrixMode(GL_PROJECTION);
				glOrtho(0, screenWidth, screenHeight, 0, -1.0, 1.0);
				glRotatef(180, 0, 0, 1);
				glTranslatef(screenWidth * -1.0f, screenHeight * -1.0f, 0);
			}
			break;

			case 3:
			{
				glViewport(screenOffsetY, windowWidth - screenWidth - screenOffsetX, screenHeight, screenWidth);
				glMatrixMode(GL_PROJECTION);
				glOrtho(0, screenHeight, screenWidth, 0, -1.0, 1.0);
				glRotatef(270, 0, 0, 1);
				glTranslatef(screenWidth * -1.0f, 0, 0);
			}
			break;
		}

		glMatrixMode(GL_MODELVIEW);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		return true;
	}

	void deinit()
	{
		destroySurface();
	}

	void swapBuffers()
	{
		SDL_GL_SwapWindow(sdlWindow);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
};
