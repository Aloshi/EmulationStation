#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include <string>
#include "Font.h"

class GuiComponent;

//The Renderer provides several higher-level functions for drawing (rectangles, text, etc.).
//Defined in multiple files - Renderer.cpp has the GuiComponent stuff, Renderer_draw_* includes renderer-specific drawing implementations, and Renderer_init_* includes renderer-specific init/deinit.
namespace Renderer
{
	void registerComponent(GuiComponent* comp);
	void unregisterComponent(GuiComponent* comp);
	void deleteAll();

	bool init(int w, int h);
	void onInit();
	void deinit();
	void onDeinit();

	void render();

	unsigned int getScreenWidth();
	unsigned int getScreenHeight();

	enum FontSize { SMALL, MEDIUM, LARGE };
	int getFontHeight(FontSize size); //sometimes font size is needed before fonts have been loaded; this takes care of that
	void buildGLColorArray(GLubyte* ptr, int color, unsigned int vertCount);

	//drawing commands
	void swapBuffers();
	void drawRect(int x, int y, int w, int h, int color);
	void drawText(std::string text, int x, int y, int color, FontSize fontsize = MEDIUM);
	void drawCenteredText(std::string text, int xOffset, int y, int color, FontSize fontsize = MEDIUM);
	void drawWrappedText(std::string text, int xStart, int yStart, int xLen, int color, FontSize fontsize = MEDIUM);
}

#endif
