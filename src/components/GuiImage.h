#ifndef _GUIIMAGE_H_
#define _GUIIMAGE_H_

#include "../GuiComponent.h"
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <string>

class GuiImage : public GuiComponent
{
public:
	GuiImage(int offsetX = 0, int offsetY = 0, std::string path = "", unsigned int maxWidth = 0, unsigned int maxHeight = 0);
	~GuiImage();

	void setImage(std::string path);

	int getWidth();
	int getHeight();

	void onRender();

	//this should really never be called by anything except setImage
	//but it was either make this function public or make mSurface public
	//so just don't use this, okay?
	int runImageLoadThread();

private:
	int mMaxWidth, mMaxHeight;

	std::string mPath, mLoadedPath;

	SDL_Surface* mSurface;
	int mOffsetX, mOffsetY;
	SDL_Rect mRect;

	SDL_Thread* mLoadThread;
	void setPathThreadSafe(std::string path);
	std::string getPathThreadSafe();
	SDL_mutex* mPathMutex;
	SDL_mutex* mSurfaceMutex;
	bool mDeleting;
};

#endif
