#include "ScrollableContainer.h"
#include "../Renderer.h"
#include "../Log.h"

ScrollableContainer::ScrollableContainer(Window* window) : GuiComponent(window), 
	mAutoScrollDelay(0), mAutoScrollSpeed(0), mAutoScrollTimer(0), mScrollPos(0, 0), mScrollDir(0, 0)
{
}

void ScrollableContainer::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	Eigen::Vector2i clipPos((int)trans.translation().x(), (int)trans.translation().y());

	Eigen::Vector3f dimScaled = trans * Eigen::Vector3f(mSize.x(), mSize.y(), 0);
	Eigen::Vector2i clipDim((int)dimScaled.x() - trans.translation().x(), (int)dimScaled.y() - trans.translation().y());

	Renderer::pushClipRect(clipPos, clipDim);

	trans.translate(Eigen::Vector3f((float)-mScrollPos.x(), (float)-mScrollPos.y(), 0));
	Renderer::setMatrix(trans);

	GuiComponent::renderChildren(trans);

	Renderer::popClipRect();
}

void ScrollableContainer::setAutoScroll(int delay, double speed)
{
	mAutoScrollDelay = delay;
	mAutoScrollSpeed = speed;
	mAutoScrollTimer = 0;
}

Eigen::Vector2d ScrollableContainer::getScrollPos() const
{
	return mScrollPos;
}

void ScrollableContainer::setScrollPos(const Eigen::Vector2d& pos)
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
			const double widthMod = (680.0 / getSize().x());
			mScrollDir = Eigen::Vector2d(0, mAutoScrollSpeed * widthMod);
		}else{
			//not enough to pass the delay, do nothing
			scrollAmt = 0;
		}
	}

	Eigen::Vector2d scroll = mScrollDir * scrollAmt;
	mScrollPos += scroll;

	//clip scrolling within bounds
	if(mScrollPos.x() < 0)
		mScrollPos[0] = 0;
	if(mScrollPos.y() < 0)
		mScrollPos[1] = 0;

	
	Eigen::Vector2f contentSize = getContentSize();
	if(mScrollPos.x() + getSize().x() > contentSize.x())
		mScrollPos[0] = (double)contentSize.x() - getSize().x();
	if(mScrollPos.y() + getSize().y() > contentSize.y())
		mScrollPos[1] = (double)contentSize.y() - getSize().y();

	GuiComponent::update(deltaTime);
}

//this should probably return a box to allow for when controls don't start at 0,0
Eigen::Vector2f ScrollableContainer::getContentSize()
{
	Eigen::Vector2f max(0, 0);
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		Eigen::Vector2f pos(mChildren.at(i)->getPosition()[0], mChildren.at(i)->getPosition()[1]);
		Eigen::Vector2f bottomRight = mChildren.at(i)->getSize() + pos;
		if(bottomRight.x() > max.x())
			max.x() = bottomRight.x();
		if(bottomRight.y() > max.y())
			max.y() = bottomRight.y();
	}

	return max;
}

void ScrollableContainer::resetAutoScrollTimer()
{
	mAutoScrollTimer = 0;
}
