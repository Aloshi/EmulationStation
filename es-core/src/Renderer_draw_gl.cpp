#include "platform.h"
#include "Renderer.h"
#include "platform_gl.h"
#include <iostream>
#include "resources/Font.h"
#include <boost/filesystem.hpp>
#include "Log.h"
#include <stack>
#include "Util.h"

namespace Renderer {
	std::stack<Eigen::Vector4i> clipStack;

	void setColor4bArray(GLubyte* array, unsigned int color)
	{
		array[0] = (color & 0xff000000) >> 24;
		array[1] = (color & 0x00ff0000) >> 16;
		array[2] = (color & 0x0000ff00) >> 8;
		array[3] = (color & 0x000000ff);
	}

	void buildGLColorArray(GLubyte* ptr, unsigned int color, unsigned int vertCount)
	{
		unsigned int colorGl;
		setColor4bArray((GLubyte*)&colorGl, color);
		for(unsigned int i = 0; i < vertCount; i++)
		{
			((GLuint*)ptr)[i] = colorGl;
		}
	}

	void pushClipRect(Eigen::Vector2i pos, Eigen::Vector2i dim)
	{
		Eigen::Vector4i box(pos.x(), pos.y(), dim.x(), dim.y());
		if(box[2] == 0)
			box[2] = Renderer::getScreenWidth() - box.x();
		if(box[3] == 0)
			box[3] = Renderer::getScreenHeight() - box.y();

		//glScissor starts at the bottom left of the window
		//so (0, 0, 1, 1) is the bottom left pixel
		//everything else uses y+ = down, so flip it to be consistent
		//rect.pos.y = Renderer::getScreenHeight() - rect.pos.y - rect.size.y;
		box[1] = Renderer::getScreenHeight() - box.y() - box[3];

		//make sure the box fits within clipStack.top(), and clip further accordingly
		if(clipStack.size())
		{
			Eigen::Vector4i& top = clipStack.top();
			if(top[0] > box[0])
				box[0] = top[0];
			if(top[1] > box[1])
				box[1] = top[1];
			if(top[0] + top[2] < box[0] + box[2])
				box[2] = (top[0] + top[2]) - box[0];
			if(top[1] + top[3] < box[1] + box[3])
				box[3] = (top[1] + top[3]) - box[1];
		}

		if(box[2] < 0)
			box[2] = 0;
		if(box[3] < 0)
			box[3] = 0;

		clipStack.push(box);
		glScissor(box[0], box[1], box[2], box[3]);
		glEnable(GL_SCISSOR_TEST);
	}

	void popClipRect()
	{
		if(clipStack.empty())
		{
			LOG(LogError) << "Tried to popClipRect while the stack was empty!";
			return;
		}

		clipStack.pop();
		if(clipStack.empty())
		{
			glDisable(GL_SCISSOR_TEST);
		}else{
			Eigen::Vector4i top = clipStack.top();
			glScissor(top[0], top[1], top[2], top[3]);
		}
	}

	void drawRect(float x, float y, float w, float h, unsigned int color, GLenum blend_sfactor, GLenum blend_dfactor)
	{
		drawRect((int)round(x), (int)round(y), (int)round(w), (int)round(h), color, blend_sfactor, blend_dfactor);
	}

	void drawRect(int x, int y, int w, int h, unsigned int color, GLenum blend_sfactor, GLenum blend_dfactor)
	{
#ifdef USE_OPENGL_ES
		GLshort points[12];
#else
		GLint points[12];
#endif

		points[0] = x; points [1] = y;
		points[2] = x; points[3] = y + h;
		points[4] = x + w; points[5] = y;

		points[6] = x + w; points[7] = y;
		points[8] = x; points[9] = y + h;
		points[10] = x + w; points[11] = y + h;

		GLubyte colors[6*4];
		buildGLColorArray(colors, color, 6);

		glEnable(GL_BLEND);
		glBlendFunc(blend_sfactor, blend_dfactor);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

#ifdef USE_OPENGL_ES
		glVertexPointer(2, GL_SHORT, 0, points);
#else
		glVertexPointer(2, GL_INT, 0, points);
#endif
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}

	void setMatrix(float* matrix)
	{
		glLoadMatrixf(matrix);
	}

	void setMatrix(const Eigen::Affine3f& matrix)
	{
		setMatrix((float*)matrix.data());
	}
};
