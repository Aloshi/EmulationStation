#include "GuiBox.h"
#include <iostream>

GuiBox::GuiBox(int offsetX, int offsetY, unsigned int width, unsigned int height)
{
	setOffsetX(offsetX);
	setOffsetY(offsetY);

	mWidth = width;
	mHeight = height;
}

void GuiBox::setData(GuiBoxData data)
{
	setBackgroundImage(data.backgroundPath, data.backgroundTiled);
	setHorizontalImage(data.horizontalPath, data.horizontalTiled);
	setVerticalImage(data.verticalPath, data.verticalTiled);
	setCornerImage(data.cornerPath);
}

void GuiBox::setHorizontalImage(std::string path, bool tiled)
{
	mHorizontalImage.setResize(12, mHeight, true);
	mHorizontalImage.setTiling(tiled);
	mHorizontalImage.setOrigin(0, 0);

	mHorizontalImage.setImage(path);
}

void GuiBox::setVerticalImage(std::string path, bool tiled)
{
	mVerticalImage.setResize(mWidth, 12, true);
	mVerticalImage.setTiling(tiled);
	mVerticalImage.setOrigin(0, 0);

	mVerticalImage.setImage(path);
}

void GuiBox::setBackgroundImage(std::string path, bool tiled)
{
	mBackgroundImage.setResize(mWidth, mHeight, true);
	mBackgroundImage.setTiling(tiled);
	mBackgroundImage.setOffsetX(getOffsetX());
	mBackgroundImage.setOffsetY(getOffsetY());

	mBackgroundImage.setImage(path);
}

void GuiBox::setCornerImage(std::string path)
{
	mCornerImage.setOrigin(0, 0);
	mCornerImage.setResize(getHorizontalBorderWidth(), getVerticalBorderWidth(), true);

	mCornerImage.setImage(path);
}

void GuiBox::onRender()
{
	//left border
	mHorizontalImage.setOffsetX(getOffsetX() - getHorizontalBorderWidth());
	mHorizontalImage.setOffsetY(getOffsetY());
	mHorizontalImage.setFlipX(false);
	mHorizontalImage.render();
	//Renderer::drawRect(getOffsetX() - getHorizontalBorderWidth(), getOffsetY(), getHorizontalBorderWidth(), mHeight, 0xFF0000);

	//right border
	mHorizontalImage.setOffsetX(getOffsetX() + mWidth);
	//same Y
	mHorizontalImage.setFlipX(true);
	mHorizontalImage.render();
	//Renderer::drawRect(getOffsetX() + mWidth, getOffsetY(), getHorizontalBorderWidth(), mHeight, 0xFF0000);

	//top border
	mVerticalImage.setOffsetX(getOffsetX());
	mVerticalImage.setOffsetY(getOffsetY() - getVerticalBorderWidth());
	mVerticalImage.setFlipY(false);
	mVerticalImage.render();
	//Renderer::drawRect(getOffsetX(), getOffsetY() - getVerticalBorderWidth(), mWidth, getVerticalBorderWidth(), 0x00FF00);

	//bottom border
	//same X
	mVerticalImage.setOffsetY(getOffsetY() + mHeight);
	mVerticalImage.setFlipY(true);
	mVerticalImage.render();
	//Renderer::drawRect(getOffsetX(), getOffsetY() + mHeight, mWidth, getVerticalBorderWidth(), 0x00FF00);


	//corner top left
	mCornerImage.setOffsetX(getOffsetX() - getHorizontalBorderWidth());
	mCornerImage.setOffsetY(getOffsetY() - getVerticalBorderWidth());
	mCornerImage.setFlipX(false);
	mCornerImage.setFlipY(false);
	mCornerImage.render();

	//top right
	mCornerImage.setOffsetX(getOffsetX() + mWidth);
	mCornerImage.setFlipX(true);
	mCornerImage.render();

	//bottom right
	mCornerImage.setOffsetY(getOffsetY() + mHeight);
	mCornerImage.setFlipY(true);
	mCornerImage.render();

	//bottom left
	mCornerImage.setOffsetX(getOffsetX() - getHorizontalBorderWidth());
	mCornerImage.setFlipX(false);
	mCornerImage.render();
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


int GuiBox::getHorizontalBorderWidth()
{
	return 12;
}

int GuiBox::getVerticalBorderWidth()
{
	return 12;
}
