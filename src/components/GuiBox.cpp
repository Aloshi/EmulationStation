#include "GuiBox.h"

GuiBox::GuiBox(Window* window, int offsetX, int offsetY, unsigned int width, unsigned int height) : GuiComponent(window), mBackgroundImage(window), 
	mHorizontalImage(window), mVerticalImage(window), mCornerImage(window)
{
	setOffset(Vector2i(offsetX, offsetY));
	
	mSize = Vector2u(width, height);
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
	mHorizontalImage.setResize(getHorizontalBorderWidth(), mSize.y, true);
}

void GuiBox::setVerticalImage(std::string path, bool tiled)
{
	mVerticalImage.setTiling(tiled);
	mVerticalImage.setOrigin(0, 0);

	mVerticalImage.setImage(path);
	mVerticalImage.setResize(mSize.x, getVerticalBorderWidth(), true);
}

void GuiBox::setBackgroundImage(std::string path, bool tiled)
{
	mBackgroundImage.setOrigin(0, 0);
	mBackgroundImage.setResize(mSize.x, mSize.y, true);
	mBackgroundImage.setTiling(tiled);
	mBackgroundImage.setOffset(0, 0);

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
	mBackgroundImage.render();

	//left border
	mHorizontalImage.setOffset(-getHorizontalBorderWidth(), 0);
	mHorizontalImage.setFlipX(false);
	mHorizontalImage.render();
	
	//right border
	mHorizontalImage.setOffset(mSize.x, 0);
	mHorizontalImage.setFlipX(true);
	mHorizontalImage.render();
	
	//top border
	mVerticalImage.setOffset(0, -getVerticalBorderWidth());
	mVerticalImage.setFlipY(false);
	mVerticalImage.render();
	
	//bottom border
	mVerticalImage.setOffset(0, mSize.y);
	mVerticalImage.setFlipY(true);
	mVerticalImage.render();


	//corner top left
	mCornerImage.setOffset(-getHorizontalBorderWidth(), -getVerticalBorderWidth());
	mCornerImage.setFlipX(false);
	mCornerImage.setFlipY(false);
	mCornerImage.render();

	//top right
	mCornerImage.setOffset(mSize.x, mCornerImage.getOffset().y);
	mCornerImage.setFlipX(true);
	mCornerImage.render();

	//bottom right
	mCornerImage.setOffset(mCornerImage.getOffset().x, mSize.y);
	mCornerImage.setFlipY(true);
	mCornerImage.render();

	//bottom left
	mCornerImage.setOffset(-getHorizontalBorderWidth(), mCornerImage.getOffset().y);
	mCornerImage.setFlipX(false);
	mCornerImage.render();

	GuiComponent::onRender();
}

void GuiBox::init()
{
	mVerticalImage.init();
	mHorizontalImage.init();
	mCornerImage.init();

	GuiComponent::init();
}

void GuiBox::deinit()
{
	mVerticalImage.deinit();
	mHorizontalImage.deinit();
	mCornerImage.deinit();

	GuiComponent::deinit();
}

int GuiBox::getHorizontalBorderWidth()
{
	return mHorizontalImage.getTextureSize().x;
}

int GuiBox::getVerticalBorderWidth()
{
	return mVerticalImage.getTextureSize().y;
}

bool GuiBox::hasBackground()
{
	return mBackgroundImage.hasImage();
}
