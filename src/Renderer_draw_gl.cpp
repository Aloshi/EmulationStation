#include "Renderer.h"
#include "platform.h"
#include GLHEADER
#include <iostream>
#include "Font.h"
#include <boost/filesystem.hpp>

namespace Renderer {
	bool loadedFonts = false;

	//defined and set in Renderer_init_rpi.cpp
	extern unsigned int display_width;
	extern unsigned int display_height;
	unsigned int getScreenWidth() { return display_width; }
	unsigned int getScreenHeight() { return display_height; }

	void setColor4bArray(GLubyte* array, int color)
	{
		array[0] = (color & 0x00ff0000) / 0x10000;
		array[1] = (color & 0x0000ff00) / 0x100;
		array[2] = (color & 0x000000ff);
		array[3] = 255;
	}

	void buildGLColorArray(GLubyte* ptr, int color, unsigned int vertCount)
	{
		for(unsigned int i = 0; i < vertCount; i++)
		{
			setColor4bArray(ptr, color);
			ptr += 4;
		}
	}

	void drawRect(int x, int y, int w, int h, int color)
	{
		GLfloat points[12];

		points[0] = x; points [1] = y;
		points[2] = x; points[3] = y + h;
		points[4] = x + w; points[5] = y;

		points[6] = x + w; points[7] = y;
		points[8] = x; points[9] = y + h;
		points[10] = x + w; points[11] = y + h;

		GLubyte colors[6*4];
		buildGLColorArray(colors, color, 6);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}




	Font* fonts[3] = {NULL, NULL, NULL};
	void loadFonts()
	{
		std::cout << "loading fonts\n";

		std::string fontPath = "LinLibertine_R.ttf";

		//make sure our font exists
		if(!boost::filesystem::exists(fontPath))
		{
			std::cout << "Default font \"" << fontPath << "\" does not exist! Attempting to default to a system font...\n";
			fontPath = "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf";
			if(!boost::filesystem::exists(fontPath))
			{
				std::cerr << "System font \"" << fontPath << "\" wasn't found either! Well, you're kind of screwed. Sorry.\n";
			}
		}

		float fontSizes[] = {0.004, 0.006, 0.009};
		for(unsigned int i = 0; i < 3; i++)
		{
			fonts[i] = new Font(fontPath, (unsigned int)(fontSizes[i] * getScreenWidth()));
		}

		loadedFonts = true;
	}

	void unloadFonts()
	{
		for(unsigned int i = 0; i < 3; i++)
		{
			delete fonts[i];
			fonts[i] = NULL;
		}

		loadedFonts = false;
	}

	Font* getFont(FontSize size)
	{
		if(!loadedFonts)
			loadFonts();

		return fonts[size];
	}


	int getFontHeight(FontSize size)
	{
		if(!loadedFonts)
			loadFonts();

		int h;
		getFont(size)->sizeText("HEIGHT", NULL, &h);

		return h;
	}

	void drawText(std::string text, int x, int y, int color, FontSize font)
	{
		if(!loadedFonts)
			loadFonts();

		//if(x < 0)
		//	std::cout << "drawing at " << x << std::endl;

		getFont(font)->drawText(text, x, y, color);
	}

	void drawCenteredText(std::string text, int xOffset, int y, int color, FontSize fontsize)
	{
		if(!loadedFonts)
			loadFonts();

		Font* font = getFont(fontsize);

		int w, h;
		font->sizeText(text, &w, &h);

		int x = (int)getScreenWidth() - w;
		x *= 0.5;

		x += xOffset * 0.5;

		drawText(text, x, y, color, fontsize);
	}

	//this could probably be optimized
	//draws text and ensures it's never longer than xLen
	void drawWrappedText(std::string text, int xStart, int yStart, int xLen, int color, FontSize fontsize)
	{
		if(!loadedFonts)
			loadFonts();

		Font* font = getFont(fontsize);

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
					drawText(line, xStart, y, color, fontsize);

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

};
