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

	void render();

	extern SDL_Surface* screen;
	extern TTF_Font* font;

	unsigned int getScreenWidth();
	unsigned int getScreenHeight();

	//drawing commands
	void drawRect(int x, int y, int w, int h, int color);
	void drawText(std::string text, int x, int y, SDL_Color& color);
	void drawCenteredText(std::string text, int y, SDL_Color& color);

	void loadFonts();
}

#endif
