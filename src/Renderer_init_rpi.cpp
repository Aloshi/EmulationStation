#include "Renderer.h"
#include <iostream>
#include "platform.h"
#include <GLES/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "Font.h"
#include <SDL/SDL.h>
#include "InputManager.h"
#include "Log.h"

#ifdef _RPI_
    #include <bcm_host.h>
#endif

namespace Renderer
{
	SDL_Surface* sdlScreen;

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig config;

#ifdef _RPI_
	static EGL_DISPMANX_WINDOW_T nativewindow;
#else
    NativeWindowType nativewindow;
#endif

	unsigned int display_width = 0;
	unsigned int display_height = 0;

	unsigned int getScreenWidth() { return display_width; }
	unsigned int getScreenHeight() { return display_height; }

	bool createSurface() //unsigned int display_width, unsigned int display_height)
	{
		LOG(LogInfo) << "Starting SDL...";

		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
		{
			LOG(LogError) << "Error initializing SDL!\n	" << SDL_GetError() << "\n" << "Are you in the 'video', 'audio', and 'input' groups? Is X closed? Is your firmware up to date? Are you using at least the 192/64 memory split?";
			return false;
		}

		sdlScreen = SDL_SetVideoMode(1, 1, 0, SDL_SWSURFACE);
		if(sdlScreen == NULL)
		{
			LOG(LogError) << "Error creating SDL window for input!";
			return false;
		}


		LOG(LogInfo) << "Creating surface...";

#ifdef _RPI_
		DISPMANX_ELEMENT_HANDLE_T dispman_element;
		DISPMANX_DISPLAY_HANDLE_T dispman_display;
		DISPMANX_UPDATE_HANDLE_T dispman_update;
		VC_RECT_T dst_rect;
		VC_RECT_T src_rect;
#endif

		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if(display == EGL_NO_DISPLAY)
		{
			LOG(LogError) << "Error getting display!";
			return false;
		}

		bool result = eglInitialize(display, NULL, NULL);
		if(result == EGL_FALSE)
		{
			LOG(LogError) << "Error initializing display!";
			return false;
		}

		result = eglBindAPI(EGL_OPENGL_ES_API);
		if(result == EGL_FALSE)
		{
			LOG(LogError) << "Error binding API!";
			return false;
		}


		static const EGLint config_attributes[] =
		{
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_NONE
		};

		GLint numConfigs;
		result = eglChooseConfig(display, config_attributes, &config, 1, &numConfigs);

		if(result == EGL_FALSE)
		{
			LOG(LogError) << "Error choosing config!";
			return false;
		}


		context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
		if(context == EGL_NO_CONTEXT)
		{
			LOG(LogError) << "Error getting context!\n	" << eglGetError();
			return false;
		}

#ifdef _RPI_
        //get hardware info for screen/desktop from BCM interface
		if(!display_width || !display_height)
		{
			bool success = graphics_get_display_size(0, &display_width, &display_height); //0 = LCD

			if(success < 0)
			{
				LOG(LogError) << "Error getting display size!";
				return false;
			}
		}
#else
        //get hardware info for screen/desktop from SDL
        if(!display_width || !display_height) 
        {
            const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
			if(videoInfo == NULL)
            {
				LOG(LogError) << "Error getting display size!";
				return false;
			}
            else
            {
                display_width = current_w;
                display_height = current_h;
            }
		}
#endif

		LOG(LogInfo) << "Resolution: " << display_width << "x" << display_height << "...";

#ifdef _RPI_
		dst_rect.x = 0; dst_rect.y = 0;
		dst_rect.width = display_width; dst_rect.height = display_height;

		src_rect.x = 0; src_rect.y = 0;
		src_rect.width = display_width << 16; src_rect.height = display_height << 16;

		dispman_display = vc_dispmanx_display_open(0); //0 = LCD
		dispman_update = vc_dispmanx_update_start(0);

		dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 0 /*layer*/, &dst_rect, 0 /*src*/, &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0 /*clamp*/, DISPMANX_NO_ROTATE /*transform*/);

		nativewindow.element = dispman_element;
        nativewindow.width = display_width; nativewindow.height = display_height;
        vc_dispmanx_update_submit_sync(dispman_update);
#endif

		surface = eglCreateWindowSurface(display, config, &nativewindow, NULL);
		if(surface == EGL_NO_SURFACE)
		{
			LOG(LogError) << "Error creating window surface!";
			return false;
		}

		result = eglMakeCurrent(display, surface, surface, context);
		if(result == EGL_FALSE)
		{
			LOG(LogError) << "Error with eglMakeCurrent!";
			return false;
		}


		LOG(LogInfo) << "Created surface successfully!";

		return true;
	}

	void swapBuffers()
	{
		eglSwapBuffers(display, surface);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void destroySurface()
	{
		eglSwapBuffers(display, surface);
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroySurface(display, surface);
		eglDestroyContext(display, context);
		eglTerminate(display);

		display = EGL_NO_DISPLAY;
		surface = EGL_NO_SURFACE;
		context = EGL_NO_CONTEXT;

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

		glViewport(0, 0, display_width, display_height);
		glMatrixMode(GL_PROJECTION);
		glOrthof(0, display_width, display_height, 0, -1.0, 1.0);
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
