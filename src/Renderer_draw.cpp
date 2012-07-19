#include "Renderer.h"
#include <SDL/SDL.h>

SDL_Surface* Renderer::screen;

void Renderer::drawRect(int x, int y, int h, int w, int color)
{
	SDL_Rect rect = {x, y, h, w};
	SDL_FillRect(Renderer::screen, &rect, color);
}
