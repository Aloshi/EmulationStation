#include "GuiImage.h"
#include <SDL/SDL_image.h>
#include <iostream>

GuiImage::GuiImage(int offsetX, int offsetY, std::string path)
{
	mSurface = NULL;
	SDL_Rect newRect = {offsetX, offsetY, 0, 0};
	mRect = newRect;

	if(!path.empty())
		setImage(path);
}

GuiImage::~GuiImage()
{
	if(mSurface)
		SDL_FreeSurface(mSurface);
}

void GuiImage::setImage(std::string path)
{
	//if we already have an image, clear it
	if(mSurface)
	{
		SDL_FreeSurface(mSurface);
		mSurface = NULL;
	}

	if(!path.empty())
	{
		mSurface = IMG_Load(path.c_str());

		if(mSurface == NULL)
			std::cerr << "Error loading image \"" << path.c_str() << "\"\n";

	}
}

void GuiImage::onRender()
{
	if(mSurface)
		SDL_BlitSurface(mSurface, NULL, Renderer::screen, &mRect);
}
