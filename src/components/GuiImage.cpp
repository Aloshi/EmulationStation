#include "GuiImage.h"
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <iostream>
#include <boost/filesystem.hpp>

int GuiImage::getWidth() { if(mSurface) return mSurface->w; else return 0; }
int GuiImage::getHeight() { if(mSurface) return mSurface->h; else return 0; }

int startImageLoadThread(void*);

GuiImage::GuiImage(int offsetX, int offsetY, std::string path, unsigned int maxWidth, unsigned int maxHeight)
{
	std::cout << "Creating GuiImage\n";

	mSurface = NULL;

	mOffsetX = offsetX;
	mOffsetY = offsetY;

	mMaxWidth = maxWidth;
	mMaxHeight = maxHeight;

	mPathMutex = SDL_CreateMutex();
	mSurfaceMutex = SDL_CreateMutex();
	mDeleting = false;

	mLoadThread = SDL_CreateThread(&startImageLoadThread, this);
	if(!mLoadThread)
	{
		std::cerr << "Error - could not create image load thread!\n";
		std::cerr << "	" << SDL_GetError() << "\n";
	}

	if(!path.empty())
		setImage(path);
}

GuiImage::~GuiImage()
{
	mDeleting = true;
	if(mLoadThread)
		SDL_WaitThread(mLoadThread, NULL);

	if(mSurface)
		SDL_FreeSurface(mSurface);
}



std::string GuiImage::getPathThreadSafe()
{
	std::string ret;

	SDL_mutexP(mPathMutex);
		ret = mPath;
	SDL_mutexV(mPathMutex);

	return ret;
}

void GuiImage::setPathThreadSafe(std::string path)
{
	SDL_mutexP(mPathMutex);
		mPath = path;

		if(mPath.empty())
			mLoadedPath = "";
	SDL_mutexV(mPathMutex);
}

int GuiImage::runImageLoadThread()
{
	while(!mDeleting)
	{
		std::string path = getPathThreadSafe();

		if(path != mLoadedPath && path != "" && boost::filesystem::exists(path))
		{
			//start loading the image
			SDL_Surface* newSurf = IMG_Load(path.c_str());

			//if we started loading something else or failed to load, don't bother resizing
			if(path != getPathThreadSafe() || newSurf == NULL)
			{
				if(newSurf)
					SDL_FreeSurface(newSurf);

				continue;
			}


			//std::cout << "Loading complete, checking for resize\n";

			//resize it
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

			//again, make sure we're still good to go
			if(path != getPathThreadSafe() || newSurf == NULL)
			{
				if(newSurf)
					SDL_FreeSurface(newSurf);

				continue;
			}

			//finally set the image and delete the old one
			SDL_mutexP(mSurfaceMutex);
				if(mSurface)
					SDL_FreeSurface(mSurface);

				mSurface = newSurf;

				//Also update the rect
				mRect.x = mOffsetX - (mSurface->w / 2);
				mRect.y = mOffsetY;
				mRect.w = 0;
				mRect.h = 0;

				mLoadedPath = path;
			SDL_mutexV(mSurfaceMutex);
		}
	}

	std::cout << "Finishing image loader thread.\n";

	return 0;
}

int startImageLoadThread(void* img)
{
	return ((GuiImage*)img)->runImageLoadThread();
}

void GuiImage::setImage(std::string path)
{
	setPathThreadSafe(path);

	if(path.empty())
	{
		if(mSurface)
		{
			SDL_mutexP(mSurfaceMutex);
				SDL_FreeSurface(mSurface);
				mSurface = NULL;
			SDL_mutexV(mSurfaceMutex);
		}
	}
}

void GuiImage::onRender()
{
	if(mSurface)
	{
		SDL_mutexP(mSurfaceMutex);
			SDL_BlitSurface(mSurface, NULL, Renderer::screen, &mRect);
		SDL_mutexV(mSurfaceMutex);
	}else if(!getPathThreadSafe().empty())
	{
		Renderer::drawCenteredText("Loading...", -(Renderer::getScreenWidth() - mOffsetX)/*-mOffsetX * 3*/, mOffsetY, 0x000000);
	}
}

