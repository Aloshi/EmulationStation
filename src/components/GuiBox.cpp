#include "GuiBox.h"

GuiBox::GuiBox(Window* window, float offsetX, float offsetY, float width, float height) : GuiComponent(window), mBackgroundImage(window), 
	mHorizontalImage(window), mVerticalImage(window), mCornerImage(window)
{
	setPosition(offsetX, offsetY);
	setSize(width, height);
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
	mHorizontalImage.setResize(mSize.x(), getHorizontalBorderWidth(), true);
}

void GuiBox::setVerticalImage(std::string path, bool tiled)
{
	mVerticalImage.setTiling(tiled);
	mVerticalImage.setOrigin(0, 0);

	mVerticalImage.setImage(path);
	mVerticalImage.setResize(getVerticalBorderWidth(), mSize.y(), true);
}

void GuiBox::setBackgroundImage(std::string path, bool tiled)
{
	mBackgroundImage.setOrigin(0, 0);
	mBackgroundImage.setResize(mSize.x(), mSize.y(), true);
	mBackgroundImage.setTiling(tiled);
	mBackgroundImage.setPosition(0, 0);

	mBackgroundImage.setImage(path);
}

void GuiBox::setCornerImage(std::string path)
{
	mCornerImage.setOrigin(0, 0);
	mCornerImage.setResize(getHorizontalBorderWidth(), getVerticalBorderWidth(), true);

	mCornerImage.setImage(path);
}

void GuiBox::setBackgroundColor(unsigned int color)
{
	mBackgroundImage.setColorShift(color);
}

void GuiBox::setBorderColor(unsigned int color)
{
	mHorizontalImage.setColorShift(color);
	mVerticalImage.setColorShift(color);
	mCornerImage.setColorShift(color);
}

void GuiBox::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	mBackgroundImage.render(trans);

	//left border
	mVerticalImage.setPosition(-getVerticalBorderWidth(), 0);
	mVerticalImage.setFlipX(false);
	mVerticalImage.render(trans);
	
	//right border
	mVerticalImage.setPosition(mSize.x(), 0);
	mVerticalImage.setFlipX(true);
	mVerticalImage.render(trans);
	
	//top border
	mHorizontalImage.setPosition(0, -getHorizontalBorderWidth());
	mHorizontalImage.setFlipY(false);
	mHorizontalImage.render(trans);
	
	//bottom border
	mHorizontalImage.setPosition(0, mSize.y());
	mHorizontalImage.setFlipY(true);
	mHorizontalImage.render(trans);


	//corner top left
	mCornerImage.setPosition(-getHorizontalBorderWidth(), -getVerticalBorderWidth());
	mCornerImage.setFlipX(false);
	mCornerImage.setFlipY(false);
	mCornerImage.render(trans);

	//top right
	mCornerImage.setPosition(mSize.x(), mCornerImage.getPosition().y());
	mCornerImage.setFlipX(true);
	mCornerImage.render(trans);

	//bottom right
	mCornerImage.setPosition(mCornerImage.getPosition().x(), mSize.y());
	mCornerImage.setFlipY(true);
	mCornerImage.render(trans);

	//bottom left
	mCornerImage.setPosition(-getHorizontalBorderWidth(), mCornerImage.getPosition().y());
	mCornerImage.setFlipX(false);
	mCornerImage.render(trans);

	GuiComponent::renderChildren(trans);
}

float GuiBox::getHorizontalBorderWidth()
{
	return (float)mHorizontalImage.getTextureSize().y();
}

float GuiBox::getVerticalBorderWidth()
{
	return (float)mVerticalImage.getTextureSize().x();
}

bool GuiBox::hasBackground()
{
	return mBackgroundImage.hasImage();
}
