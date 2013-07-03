#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include <string>
#include "Vector2.h"
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

	//just takes care of default font init/deinit right now
	void onInit();
	void onDeinit();

	unsigned int getScreenWidth();
	unsigned int getScreenHeight();

	void buildGLColorArray(GLubyte* ptr, unsigned int color, unsigned int vertCount);

	//graphics commands
	void swapBuffers();

	void translatef(float x, float y);
	void translate(Vector2i offset);
	
	void pushClipRect(int x, int y, unsigned int w, unsigned int h);
	void pushClipRect(Vector2i offset, Vector2u size);
	void popClipRect();

	void drawRect(int x, int y, int w, int h, unsigned int color);
}

#endif
