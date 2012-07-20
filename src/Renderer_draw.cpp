#include "Renderer.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <iostream>

SDL_Surface* Renderer::screen;

TTF_Font* Renderer::font;

void Renderer::drawRect(int x, int y, int h, int w, int color)
{
	SDL_Rect rect = {x, y, h, w};
	SDL_FillRect(Renderer::screen, &rect, color);
}

void Renderer::loadFonts()
{
	font = TTF_OpenFont("LinLibertine_R.ttf", 72);
	if(!font)
	{
		std::cerr << "Error - could not load font!\n";
		std::cerr << TTF_GetError() << "\n";
		return;
	}
}

void Renderer::drawText(std::string text, int x, int y, SDL_Color& color)
{
	if(!font)
		loadFonts();

	//color = {255, 0, 0};

	SDL_Surface* textSurf = TTF_RenderText_Blended(font, text.c_str(), color);
	if(textSurf == NULL)
	{
		std::cerr << "Error - could not render text \"" << text << "\" to surface!\n";
		std::cerr << TTF_GetError() << "\n";
		return;
	}

	SDL_Rect dest = {x, y};
	SDL_BlitSurface(textSurf, NULL, screen, &dest);
	SDL_FreeSurface(textSurf);
}

void Renderer::drawCenteredText(std::string text, int y, SDL_Color& color)
{
	if(!font)
		loadFonts();

	int w, h;
	TTF_SizeText(font, text.c_str(), &w, &h);

	int x = (int)getScreenWidth() - w;
	x *= 0.5;

	drawText(text, x, y, color);
}
