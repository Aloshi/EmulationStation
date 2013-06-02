#include "GuiBox.h"

GuiBox::GuiBox(Window* window, int offsetX, int offsetY, unsigned int width, unsigned int height) : GuiComponent(window), mBackgroundImage(window), 
	mHorizontalImage(window), mVerticalImage(window), mCornerImage(window)
{
	setOffset(Vector2i(offsetX, offsetY));
	
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
	mHorizontalImage.setTiling(tiled);
	mHorizontalImage.setOrigin(0, 0);

	mHorizontalImage.setImage(path);
	mHorizontalImage.setResize(mHorizontalImage.getHeight(), mHeight, true);
}

void GuiBox::setVerticalImage(std::string path, bool tiled)
{
	mVerticalImage.setTiling(tiled);
	mVerticalImage.setOrigin(0, 0);

	mVerticalImage.setImage(path);
	mVerticalImage.setResize(mWidth, mVerticalImage.getHeight(), true);
}

void GuiBox::setBackgroundImage(std::string path, bool tiled)
{
	mBackgroundImage.setOrigin(0, 0);
	mBackgroundImage.setResize(mWidth, mHeight, true);
	mBackgroundImage.setTiling(tiled);
	mBackgroundImage.setOffset(getOffset());

	mBackgroundImage.setImage(path);
}

void GuiBox::setCornerImage(std::string path)
{
	mCornerImage.setOrigin(0, 0);
	mCornerImage.setResize(getHorizontalBorderWidth(), getVerticalBorderWidth(), true);

	mCornerImage.setImage(path);
}

void GuiBox::render()
{
	mBackgroundImage.render();

	//left border
	mHorizontalImage.setOffset(getOffset().x - getHorizontalBorderWidth(), getOffset().y);
	mHorizontalImage.setFlipX(false);
	mHorizontalImage.render();
	
	//right border
	mHorizontalImage.setOffset(getOffset().x + mWidth, getOffset().y);
	mHorizontalImage.setFlipX(true);
	mHorizontalImage.render();
	
	//top border
	mVerticalImage.setOffset(getOffset().x, getOffset().y - getVerticalBorderWidth());
	mVerticalImage.setFlipY(false);
	mVerticalImage.render();
	
	//bottom border
	mVerticalImage.setOffset(getOffset().x, getOffset().y + mHeight);
	mVerticalImage.setFlipY(true);
	mVerticalImage.render();


	//corner top left
	mCornerImage.setOffset(getOffset().x - getHorizontalBorderWidth(), getOffset().y - getVerticalBorderWidth());
	mCornerImage.setFlipX(false);
	mCornerImage.setFlipY(false);
	mCornerImage.render();

	//top right
	mCornerImage.setOffset(getOffset().x + mWidth, mCornerImage.getOffset().y);
	mCornerImage.setFlipX(true);
	mCornerImage.render();

	//bottom right
	mCornerImage.setOffset(mCornerImage.getOffset().x, getOffset().y + mHeight);
	mCornerImage.setFlipY(true);
	mCornerImage.render();

	//bottom left
	mCornerImage.setOffset(getOffset().x - getHorizontalBorderWidth(), mCornerImage.getOffset().y);
	mCornerImage.setFlipX(false);
	mCornerImage.render();
}

void GuiBox::init()
{
	mVerticalImage.init();
	mHorizontalImage.init();
	mCornerImage.init();
}

void GuiBox::deinit()
{
	mVerticalImage.deinit();
	mHorizontalImage.deinit();
	mCornerImage.deinit();
}

int GuiBox::getHorizontalBorderWidth()
{
	return mHorizontalImage.getWidth();
}

int GuiBox::getVerticalBorderWidth()
{
	return mVerticalImage.getHeight();
}

bool GuiBox::hasBackground()
{
	return mBackgroundImage.hasImage();
}
