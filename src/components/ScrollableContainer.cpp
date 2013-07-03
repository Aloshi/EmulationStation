#include "ScrollableContainer.h"
#include "../Renderer.h"
#include "../Log.h"

ScrollableContainer::ScrollableContainer(Window* window) : GuiComponent(window), 
	mAutoScrollDelay(0), mAutoScrollSpeed(0), mAutoScrollTimer(0)
{
}

void ScrollableContainer::render()
{
	Renderer::pushClipRect(getGlobalOffset(), getSize());

	Vector2f translate = (Vector2f)mOffset - (Vector2f)mScrollPos;

	Renderer::translatef(translate.x, translate.y);
	
	GuiComponent::onRender();

	Renderer::translatef(-translate.x, -translate.y);

	Renderer::popClipRect();
}

void ScrollableContainer::setAutoScroll(int delay, double speed)
{
	mAutoScrollDelay = delay;
	mAutoScrollSpeed = speed;
	mAutoScrollTimer = 0;
}

Vector2d ScrollableContainer::getScrollPos() const
{
	return mScrollPos;
}

void ScrollableContainer::setScrollPos(const Vector2d& pos)
{
	mScrollPos = pos;
}

void ScrollableContainer::update(int deltaTime)
{
	double scrollAmt = (double)deltaTime;

	if(mAutoScrollSpeed != 0)
	{
		mAutoScrollTimer += deltaTime;

		scrollAmt = (float)(mAutoScrollTimer - mAutoScrollDelay);

		if(scrollAmt > 0)
		{
			//scroll the amount of time left over from the delay
			mAutoScrollTimer = mAutoScrollDelay;

			//scale speed by our width! more text per line = slower scrolling
			const double widthMod = (680.0 / getSize().x);
			mScrollDir = Vector2d(0, mAutoScrollSpeed * widthMod);
		}else{
			//not enough to pass the delay, do nothing
			scrollAmt = 0;
		}
	}

	Vector2d scroll = mScrollDir * scrollAmt;
	mScrollPos += scroll;

	//clip scrolling within bounds
	if(mScrollPos.x < 0)
		mScrollPos.x = 0;
	if(mScrollPos.y < 0)
		mScrollPos.y = 0;

	
	Vector2i contentSize = getContentSize();
	if(mScrollPos.x + getSize().x > contentSize.x)
		mScrollPos.x = (double)contentSize.x - getSize().x;
	if(mScrollPos.y + getSize().y > contentSize.y)
		mScrollPos.y = (double)contentSize.y - getSize().y;

	GuiComponent::update(deltaTime);
}

//this should probably return a box to allow for when controls don't start at 0,0
Vector2i ScrollableContainer::getContentSize()
{
	Vector2i max;
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		Vector2i bottomRight = (Vector2i)mChildren.at(i)->getSize() + mChildren.at(i)->getOffset();
		if(bottomRight.x > max.x)
			max.x = bottomRight.x;
		if(bottomRight.y > max.y)
			max.y = bottomRight.y;
	}

	return max;
}

void ScrollableContainer::setSize(Vector2u size)
{
	mSize = size;
}

void ScrollableContainer::resetAutoScrollTimer()
{
	mAutoScrollTimer = 0;
}
