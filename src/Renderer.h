#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include <string>
#include "Font.h"

class GuiComponent;

namespace Renderer
{
	void registerComponent(GuiComponent* comp);
	void unregisterComponent(GuiComponent* comp);
	void deleteAll();

	bool init(int w, int h);
	void deinit();

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
