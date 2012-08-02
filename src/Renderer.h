#ifndef _RENDERER_H_
#define _RENDERER_H_

#define LAYER_COUNT 3

#define BIT(x) (1 << (x))

#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <string>

class GuiComponent;

namespace Renderer
{
	void registerComponent(GuiComponent* comp);
	void unregisterComponent(GuiComponent* comp);
	void deleteAll();

	void render();

	extern SDL_Surface* screen;
	extern TTF_Font* font;

	unsigned int getScreenWidth();
	unsigned int getScreenHeight();

	enum FontSize { SMALL, MEDIUM, LARGE };
	bool loadFonts();
	extern bool loadedFonts;
	extern TTF_Font* fonts[3]; //should be FontSize size but I don't remember the syntax
	extern int fontHeight[3]; //same
	int getFontHeight(FontSize size); //sometimes font size is needed before fonts have been loaded; this takes care of that

	//drawing commands
	void drawRect(int x, int y, int w, int h, int color);
	void drawText(std::string text, int x, int y, int color, FontSize fontsize = MEDIUM);
	void drawCenteredText(std::string text, int xOffset, int y, int color, FontSize fontsize = MEDIUM);
	void drawWrappedText(std::string text, int xStart, int yStart, int xLen, int color, FontSize fontsize = MEDIUM);
}

#endif
