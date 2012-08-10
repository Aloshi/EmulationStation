#include "GuiImage.h"
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <iostream>
#include <boost/filesystem.hpp>

int GuiImage::getWidth() { if(mSurface) return mSurface->w; else return 0; }
int GuiImage::getHeight() { if(mSurface) return mSurface->h; else return 0; }

GuiImage::GuiImage(int offsetX, int offsetY, std::string path, unsigned int maxWidth, unsigned int maxHeight, bool resizeExact)
{
	mSurface = NULL;

	mOffsetX = offsetX;
	mOffsetY = offsetY;

	mMaxWidth = maxWidth;
	mMaxHeight = maxHeight;
	mResizeExact = resizeExact;

	if(!path.empty())
		setImage(path);
}

GuiImage::~GuiImage()
{
	if(mSurface)
		SDL_FreeSurface(mSurface);
}

void GuiImage::loadImage(std::string path)
{
	if(boost::filesystem::exists(path))
	{
		//start loading the image
		SDL_Surface* newSurf = IMG_Load(path.c_str());

		//if we started loading something else or failed to load, don't bother resizing
		if(newSurf == NULL)
		{
			std::cerr << "Error loading image.\n";
			return;
		}


		//resize it
		if(mResizeExact)
		{
			double scaleX = (double)mMaxWidth / (double)newSurf->w;
			double scaleY = (double)mMaxHeight / (double)newSurf->h;

			SDL_Surface* resSurf = zoomSurface(newSurf, scaleX, scaleY, SMOOTHING_OFF);
			SDL_FreeSurface(newSurf);
			newSurf = resSurf;
		}else{
			if(mMaxWidth && newSurf->w > mMaxWidth)
			{
				double scale = (double)mMaxWidth / (double)newSurf->w;

				SDL_Surface* resSurf = zoomSurface(newSurf, scale, scale, SMOOTHING_OFF);
				SDL_FreeSurface(newSurf);
				newSurf = resSurf;
        		}

			if(mMaxHeight && newSurf->h > mMaxHeight)
		      	{
				double scale = (double)mMaxHeight / (double)newSurf->h;

				SDL_Surface* resSurf = zoomSurface(newSurf, scale, scale, SMOOTHING_OFF);
				SDL_FreeSurface(newSurf);
				newSurf = resSurf;
			}
		}


		//convert it into display format for faster rendering
		SDL_Surface* dispSurf = SDL_DisplayFormat(newSurf);
		SDL_FreeSurface(newSurf);
		newSurf = dispSurf;


		//finally set the image and delete the old one
		if(mSurface)
			SDL_FreeSurface(mSurface);

		mSurface = newSurf;

		//Also update the rect
		mRect.x = mOffsetX - (mSurface->w / 2);
		mRect.y = mOffsetY;
		mRect.w = mSurface->w;
		mRect.h = mSurface->h;
	}else{
		std::cerr << "File \"" << path << "\" not found!\n";
	}
}

void GuiImage::setImage(std::string path)
{
	if(mPath == path)
		return;

	mPath = path;

	if(mSurface)
	{
		SDL_FreeSurface(mSurface);
		mSurface = NULL;
	}

	if(!path.empty())
		loadImage(path);

}

void GuiImage::onRender()
{
	if(mSurface)
		SDL_BlitSurface(mSurface, NULL, Renderer::screen, &mRect);
}
