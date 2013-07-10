#ifndef _GUIBOX_H_
#define _GUIBOX_H_

#include "../GuiComponent.h"
#include "ImageComponent.h"
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

class GuiBox : public GuiComponent
{
public:
	GuiBox(Window* window, float offsetX, float offsetY, float width, float height);

	void setData(GuiBoxData data);

	void setBackgroundImage(std::string path, bool tiled = true);
	void setHorizontalImage(std::string path, bool tiled = false);
	void setVerticalImage(std::string path, bool tiled = false);
	void setCornerImage(std::string path);

	bool hasBackground();

	void render(const Eigen::Affine3f& parentTrans) override;

private:
	ImageComponent mBackgroundImage, mHorizontalImage, mVerticalImage, mCornerImage;

	float getHorizontalBorderWidth();
	float getVerticalBorderWidth();
};

#endif
