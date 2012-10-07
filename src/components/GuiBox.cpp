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
	mCornerImage.setResize(getHorizontalBorderWidth(), getVerticalBorderWidth(), true);
}

void GuiBox::onRender()
{
	//left border
	mHorizontalImage.setOffsetX(getOffsetX() - getHorizontalBorderWidth());
	mHorizontalImage.setOffsetY(getOffsetY());
	mHorizontalImage.render();
	//Renderer::drawRect(getOffsetX() - getHorizontalBorderWidth(), getOffsetY(), getHorizontalBorderWidth(), mHeight, 0xFF0000);

	//right border
	mHorizontalImage.setOffsetX(getOffsetX() + mWidth);
	//same Y
	mHorizontalImage.render();
	//Renderer::drawRect(getOffsetX() + mWidth, getOffsetY(), getHorizontalBorderWidth(), mHeight, 0xFF0000);

	//top border
	mVerticalImage.setOffsetX(getOffsetX());
	mVerticalImage.setOffsetY(getOffsetY() - getVerticalBorderWidth());
	mVerticalImage.render();
	//Renderer::drawRect(getOffsetX(), getOffsetY() - getVerticalBorderWidth(), mWidth, getVerticalBorderWidth(), 0x00FF00);

	//bottom border
	//same X
	mVerticalImage.setOffsetY(getOffsetY() + mHeight);
	mVerticalImage.render();
	//Renderer::drawRect(getOffsetX(), getOffsetY() + mHeight, mWidth, getVerticalBorderWidth(), 0x00FF00);

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
