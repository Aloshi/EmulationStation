#ifndef _GUIBOX_H_
#define _GUIBOX_H_

#include "../Gui.h"
#include "GuiImage.h"
#include <string>

struct GuiBoxData
{
	std::string backgroundPath;
	bool backgroundTiled;
	std::string horizontalPath;
	bool horizontalTiled;
	std::string verticalPath;
	bool verticalTiled;
	std::string cornerPath;
};

class GuiBox : public Gui
{
public:
	GuiBox(Window* window, int offsetX, int offsetY, unsigned int width, unsigned int height);

	void setData(GuiBoxData data);

	void setBackgroundImage(std::string path, bool tiled = true);
	void setHorizontalImage(std::string path, bool tiled = false);
	void setVerticalImage(std::string path, bool tiled = false);
	void setCornerImage(std::string path);

	bool hasBackground();

	void render();

	void onInit();
	void onDeinit();
private:
	GuiImage mBackgroundImage, mHorizontalImage, mVerticalImage, mCornerImage;

	int getHorizontalBorderWidth();
	int getVerticalBorderWidth();

	unsigned int mWidth, mHeight;
};

#endif
