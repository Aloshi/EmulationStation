#ifndef _GUIIMAGE_H_
#define _GUIIMAGE_H_

#include "../GuiComponent.h"
#include <SDL/SDL.h>
#include <string>

class GuiImage : public GuiComponent
{
public:
	GuiImage(int offsetX = 0, int offsetY = 0, std::string path = "");
	~GuiImage();

	void setImage(std::string path);

	void onRender();
private:
	SDL_Surface* mSurface;
	SDL_Rect mRect;
};

#endif
