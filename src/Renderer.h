#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include <string>
#include "platform.h"
#include GLHEADER
//#include "Font.h"

class GuiComponent;
class Font;

//The Renderer provides several higher-level functions for drawing (rectangles, text, etc.).
//Defined in multiple files - Renderer.cpp has the GuiComponent stuff, Renderer_draw_* includes renderer-specific drawing implementations, and Renderer_init_* includes renderer-specific init/deinit.
namespace Renderer
{
	bool init(int w, int h);
	void deinit();

	unsigned int getScreenWidth();
	unsigned int getScreenHeight();

	enum FontSize { SMALL, MEDIUM, LARGE };
	Font* getDefaultFont(FontSize size);
	void buildGLColorArray(GLubyte* ptr, unsigned int color, unsigned int vertCount);

	//drawing commands
	void swapBuffers();
	void drawRect(int x, int y, int w, int h, unsigned int color);
	void drawText(std::string text, int x, int y, unsigned int color, Font* font);
	void drawCenteredText(std::string text, int xOffset, int y, unsigned int color, Font* font);
	void drawWrappedText(std::string text, int xStart, int yStart, int xLen, unsigned int color, Font* font);
}

#endif
