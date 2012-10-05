#include "GuiBox.h"

GuiBox::GuiBox(int offsetX, int offsetY, unsigned int width, unsigned int height)
{
	setOffsetX(offsetX);
	setOffsetY(offsetY);

	mWidth = width;
	mHeight = height;
}

void GuiBox::setHorizontalImage(std::string path, bool tiled)
{
	mHorizontalImage.setImage(path);
}

void GuiBox::setVerticalImage(std::string path, bool tiled)
{
	mVerticalImage.setImage(path);
}

void GuiBox::setBackgroundImage(std::string path, bool tiled)
{
	mBackgroundImage.setImage(path);
	mBackgroundImage.setTiling(tiled);
}

void GuiBox::onRender()
{
	//left border
	mHorizontalImage.setOffsetX(getOffsetX());
	mHorizontalImage.setOffsetY(getOffsetY());
	mHorizontalImage.setOrigin(0.5, 0);
	mHorizontalImage.setResize(12, mHeight, true);
	mHorizontalImage.render();

	//right border
	mHorizontalImage.setOffsetX(getOffsetX() + mWidth);
	mHorizontalImage.setOffsetY(getOffsetY());
	mHorizontalImage.render();
}

void GuiBox::onInit()
{
	mHorizontalImage.init();
	mVerticalImage.init();
}

void GuiBox::onDeinit()
{
	mHorizontalImage.deinit();
	mVerticalImage.deinit();
}
