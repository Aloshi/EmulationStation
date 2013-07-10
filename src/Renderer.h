#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include <string>
#include "platform.h"
#include "Eigen/Dense"
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

	//doesn't do anything right now
	void onInit();
	void onDeinit();

	unsigned int getScreenWidth();
	unsigned int getScreenHeight();

	void buildGLColorArray(GLubyte* ptr, unsigned int color, unsigned int vertCount);

	//graphics commands
	void swapBuffers();

	void pushClipRect(Eigen::Vector2i pos, Eigen::Vector2i dim);
	void popClipRect();

	void setMatrix(float* mat);
	void setMatrix(const Eigen::Affine3f& transform);

	void drawRect(int x, int y, int w, int h, unsigned int color);
}

#endif
