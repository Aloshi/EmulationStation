#ifndef _GUIBOX_H_
#define _GUIBOX_H_

#include "../GuiComponent.h"
#include "GuiImage.h"
#include <string>

class GuiBox : public GuiComponent
{
public:
	GuiBox(int offsetX, int offsetY, unsigned int width, unsigned int height);

	void setBackgroundImage(std::string path, bool tiled = true);
	void setHorizontalImage(std::string path, bool tiled = false);
	void setVerticalImage(std::string path, bool tiled = false);

	void onRender();

	void onInit();
	void onDeinit();
private:
	GuiImage mBackgroundImage, mHorizontalImage, mVerticalImage;

	unsigned int mWidth, mHeight;
};

#endif
