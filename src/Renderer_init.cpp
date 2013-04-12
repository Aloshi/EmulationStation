#include "platform.h"

#ifdef _RPI_
	#include "Renderer_init_rpi.cpp"
#endif

#ifdef _DESKTOP_
	#include "Renderer_init_sdlgl.cpp"
#endif

namespace Renderer
{
	void onInit()
	{
		for(int i = 0; i < (int)FONT_SIZE_COUNT; i++)
		{
			getDefaultFont((FontSize)i)->init();
		}
	}

	void onDeinit()
	{
		for(int i = 0; i < (int)FONT_SIZE_COUNT; i++)
		{
			getDefaultFont((FontSize)i)->deinit();
		}
	}
};
