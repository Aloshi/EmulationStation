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

	//default origin (center of image)
	mOriginX = 0.5;
	mOriginY = 0.5;

	mTiled = false;

	mMaxWidth = maxWidth;
	mMaxHeight = maxHeight;

	mResizeExact = resizeExact;
	mUseAlpha = false;

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


		resizeSurface(&newSurf);

		//convert it into display format for faster rendering
		SDL_Surface* dispSurf;
		if(mUseAlpha)
			dispSurf = SDL_DisplayFormatAlpha(newSurf);
		else
 			dispSurf = SDL_DisplayFormat(newSurf);

		SDL_FreeSurface(newSurf);
		newSurf = dispSurf;


		//finally set the image and delete the old one
		if(mSurface)
			SDL_FreeSurface(mSurface);

		mSurface = newSurf;

		updateRect();

	}else{
		std::cerr << "File \"" << path << "\" not found!\n";
	}
}

//enjoy this overly-complicated pointer stuff that results from splitting a function too late
void GuiImage::resizeSurface(SDL_Surface** surfRef)
{
	if(mTiled)
		return;

	SDL_Surface* newSurf = *surfRef;
	if(mResizeExact)
	{
		double scaleX = (double)mMaxWidth / (double)newSurf->w;
		double scaleY = (double)mMaxHeight / (double)newSurf->h;

		if(scaleX == 0)
			scaleX = scaleY;
		if(scaleY == 0)
			scaleY = scaleX;

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

	*surfRef = newSurf;
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

void GuiImage::updateRect()
{
	mRect.x = mOffsetX /*- mSurface->w*/ - (mSurface->w * mOriginX);
	mRect.y = mOffsetY - (mSurface->h * mOriginY);
	mRect.w = mSurface->w;
	mRect.h = mSurface->h;
}

void GuiImage::setOrigin(float originX, float originY)
{
	mOriginX = originX;
	mOriginY = originY;

	if(mSurface)
		updateRect();
}

void GuiImage::setTiling(bool tile)
{
	mTiled = tile;

	if(mTiled)
		mResizeExact = false;
}

void GuiImage::setAlpha(bool useAlpha)
{
	mUseAlpha = useAlpha;

	if(mSurface)
	{
		SDL_FreeSurface(mSurface);
		mSurface = NULL;
		loadImage(mPath);
	}
}

bool dbg = false;

void GuiImage::onRender()
{
	if(mSurface)
	{
		if(mTiled)
		{
			SDL_Rect rect = mRect;
			for(int x = 0; x < mMaxWidth / mSurface->w + 0.5; x++)
			{
				for(int y = 0; y < mMaxHeight / mSurface->h + 0.5; y++)
				{
					SDL_BlitSurface(mSurface, NULL, Renderer::screen, &rect);
					rect.y += mSurface->h;
				}
				rect.x += mSurface->w;
				rect.y = mRect.y;
			}
		}else{
			SDL_BlitSurface(mSurface, NULL, Renderer::screen, &mRect);
		}
	}
}
