#include "Renderer.h"
#include "platform.h"
#include GLHEADER
#include "Font.h"

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
