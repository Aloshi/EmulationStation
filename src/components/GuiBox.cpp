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
	mHorizontalImage.setResize(getHorizontalBorderWidth(), mSize.y(), true);
}

void GuiBox::setVerticalImage(std::string path, bool tiled)
{
	mVerticalImage.setTiling(tiled);
	mVerticalImage.setOrigin(0, 0);

	mVerticalImage.setImage(path);
	mVerticalImage.setResize(mSize.x(), getVerticalBorderWidth(), true);
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

void GuiBox::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	mBackgroundImage.render(trans);

	//left border
	mHorizontalImage.setPosition(-getHorizontalBorderWidth(), 0);
	mHorizontalImage.setFlipX(false);
	mHorizontalImage.render(trans);
	
	//right border
	mHorizontalImage.setPosition(mSize.x(), 0);
	mHorizontalImage.setFlipX(true);
	mHorizontalImage.render(trans);
	
	//top border
	mVerticalImage.setPosition(0, -getVerticalBorderWidth());
	mVerticalImage.setFlipY(false);
	mVerticalImage.render(trans);
	
	//bottom border
	mVerticalImage.setPosition(0, mSize.y());
	mVerticalImage.setFlipY(true);
	mVerticalImage.render(trans);


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
	return (float)mHorizontalImage.getTextureSize().x();
}

float GuiBox::getVerticalBorderWidth()
{
	return (float)mVerticalImage.getTextureSize().y();
}

bool GuiBox::hasBackground()
{
	return mBackgroundImage.hasImage();
}
