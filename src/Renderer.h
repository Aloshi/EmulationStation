#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <vector>
#include <string>
#include "platform.h"
#include <Eigen/Dense>
#include GLHEADER
//#include "Font.h"

class GuiComponent;
class Font;

//The Renderer provides several higher-level functions for drawing (rectangles, text, etc.).
//Renderer_draw_gl.cpp has most of the higher-level functions and wrappers.
//Renderer_init_*.cpp has platform-specific renderer initialziation/deinitialziation code.  (e.g. the Raspberry Pi sets up dispmanx/OpenGL ES)
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
