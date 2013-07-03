#include "platform.h"
#include "Renderer.h"
#include GLHEADER
#include <iostream>
#include "Font.h"
#include <boost/filesystem.hpp>
#include "Log.h"
#include <stack>

namespace Renderer {
	bool loadedFonts = false;

	std::stack<Rect> clipStack;

	void setColor4bArray(GLubyte* array, unsigned int color)
	{
		array[0] = (color & 0xff000000) >> 24;
		array[1] = (color & 0x00ff0000) >> 16;
		array[2] = (color & 0x0000ff00) >> 8;
		array[3] = (color & 0x000000ff);
	}

	void buildGLColorArray(GLubyte* ptr, unsigned int color, unsigned int vertCount)
	{
		//convert color from ???? to RGBA?
		unsigned int colorRGBA;
		setColor4bArray((GLubyte *)&colorRGBA, color);
		//write color to unsigned int array
		GLuint * uiPtr = (GLuint *)ptr;
		for(unsigned int i = 0; i < vertCount; i++)
		{
			uiPtr[i] = colorRGBA;
		}
	}

	void translatef(float x, float y)
	{
		glTranslatef(x, y, 0);
	}

	void translate(Vector2i offset)
	{
		translatef((float)offset.x, (float)offset.y);
	}

	void pushClipRect(int x, int y, unsigned int w, unsigned int h)
	{
		Rect rect(x, y, w, h);
		if(rect.size.x == 0)
			rect.size.x = Renderer::getScreenWidth() - rect.pos.x;
		if(rect.size.y == 0)
			rect.size.y = Renderer::getScreenHeight() - rect.pos.y;

		//glScissor starts at the bottom left of the window
		//so (0, 0, 1, 1) is the bottom left pixel
		//everything else uses y+ = down, so flip it to be consistent
		rect.pos.y = Renderer::getScreenHeight() - rect.pos.y - rect.size.y;

		clipStack.push(rect);
		glScissor(rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
		glEnable(GL_SCISSOR_TEST);
	}

	void pushClipRect(Vector2i pos, Vector2u size)
	{
		pushClipRect(pos.x, pos.y, size.x, size.y);
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
			Rect top = clipStack.top();
			glScissor(top.pos.x, top.pos.y, top.size.x, top.size.y);
		}
	}

	void drawRect(int x, int y, int w, int h, unsigned int color)
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
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

#ifdef USE_OPENGL_ES
		glVertexPointer(2, GL_SHORT, 0, points);
#else
		glVertexPointer(2, GL_INT, 0, points);
#endif
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableClientState(GL_BLEND);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_COLOR_ARRAY);
	}



	Font* fonts[3] = {NULL, NULL, NULL};

	/*void unloadFonts()
	{
		std::cout << "unloading fonts...";

		for(unsigned int i = 0; i < 3; i++)
		{
			delete fonts[i];
			fonts[i] = NULL;
		}

		loadedFonts = false;

		std::cout << "done.\n";
	}*/

	//creates the default fonts (which shouldn't ever be deleted)
	void loadFonts()
	{
		if(loadedFonts)
			return;

		std::string fontPath = Font::getDefaultPath();

		//make sure our font exists
		if(!boost::filesystem::exists(fontPath))
		{
			LOG(LogError) << "System font wasn't found! Try installing the DejaVu truetype font (pacman -S ttf-dejavu on Arch, sudo apt-get install ttf-dejavu on Debian)";
			return;
		}

		float fontSizes[] = {0.035f, 0.045f, 0.1f};
		for(unsigned int i = 0; i < 3; i++)
		{
			fonts[i] = new Font(fontPath, (unsigned int)(fontSizes[i] * getScreenHeight()));
		}

		loadedFonts = true;

		LOG(LogInfo) << "Loaded fonts successfully.";
	}

	Font* getDefaultFont(FontSize size)
	{
		if(!loadedFonts)
			loadFonts();

		return fonts[size];
	}

	void drawText(std::string text, int x, int y, unsigned int color, Font* font)
	{
		font->drawText(text, x, y, color);
	}

	void drawCenteredText(std::string text, int xOffset, int y, unsigned int color, Font* font)
	{
		int w, h;
		font->sizeText(text, &w, &h);

		int x = getScreenWidth() - w;
		x = x / 2;
		x += xOffset / 2;

		drawText(text, x, y, color, font);
	}

	//this could probably be optimized
	//draws text and ensures it's never longer than xLen
	void drawWrappedText(std::string text, int xStart, int yStart, int xLen, unsigned int color, Font* font)
	{
		int y = yStart;

		std::string line, word, temp;
		int w, h;
		size_t space, newline;

		while(text.length() > 0 || !line.empty()) //while there's text or we still have text to render
		{
			space = text.find(' ', 0);
			if(space == std::string::npos)
				space = text.length() - 1;


			word = text.substr(0, space + 1);

			//check if the next word contains a newline
			newline = word.find('\n', 0);
			if(newline != std::string::npos)
			{
				word = word.substr(0, newline);
				text.erase(0, newline + 1);
			}else{
				text.erase(0, space + 1);
			}

			temp = line + word;

			font->sizeText(temp, &w, &h);

			//if we're on the last word and it'll fit on the line, just add it to the line
			if((w <= xLen && text.length() == 0) || newline != std::string::npos)
			{
				line = temp;
				word = "";
			}


			//if the next line will be too long or we're on the last of the text, render it
			if(w > xLen || text.length() == 0 || newline != std::string::npos)
			{
				//render line now
				if(w > 0) //make sure it's not blank
					drawText(line, xStart, y, color, font);

				//increment y by height and some extra padding for the next line
				y += h + 4;

				//move the word we skipped to the next line
				line = word;
			}else{
				//there's still space, continue building the line
				line = temp;
			}

		}
	}

	void sizeWrappedText(std::string text, int xLen, Font* font, int* xOut, int* yOut)
	{
		if(xOut != NULL)
			*xOut = xLen;

		int y = 0;

		std::string line, word, temp;
		int w, h;
		size_t space, newline;

		while(text.length() > 0 || !line.empty()) //while there's text or we still have text to render
		{
			space = text.find(' ', 0);
			if(space == std::string::npos)
				space = text.length() - 1;

			word = text.substr(0, space + 1);

			//check if the next word contains a newline
			newline = word.find('\n', 0);
			if(newline != std::string::npos)
			{
				word = word.substr(0, newline);
				text.erase(0, newline + 1);
			}else{
				text.erase(0, space + 1);
			}

			temp = line + word;

			font->sizeText(temp, &w, &h);

			//if we're on the last word and it'll fit on the line, just add it to the line
			if((w <= xLen && text.length() == 0) || newline != std::string::npos)
			{
				line = temp;
				word = "";
			}

			//if the next line will be too long or we're on the last of the text, render it
			if(w > xLen || text.length() == 0 || newline != std::string::npos)
			{
				//increment y by height and some extra padding for the next line
				y += h + 4;

				//move the word we skipped to the next line
				line = word;
			}else{
				//there's still space, continue building the line
				line = temp;
			}

		}

		if(yOut != NULL)
			*yOut = y;
	}

};
