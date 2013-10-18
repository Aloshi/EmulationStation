#include "VerticalImageAutoScrollbox.h"
#include "../Renderer.h"
#include "../Log.h"
#include <boost/math/constants/constants.hpp>
#include <cmath>

VerticalImageAutoScrollbox::VerticalImageAutoScrollbox(Window* window)
        : GuiComponent(window)
        , mScrollPos(0, 0)
        , mAutoScrollDelay(0)
        , mAnimDuration(0)
        , mAutoScrollTimer(0)
        , mBorderSpace(0.f)
        , mAllowUpscaling(true)
        , mCentered(true)
        , mAnimTargetChildNo(0)
{
}

void VerticalImageAutoScrollbox::setAutoScroll(int delay, int animTime)
{
	mAutoScrollDelay = delay;
	mAnimDuration = animTime;
	mAutoScrollTimer = 0;
        if (getChildCount() > 1)
        {
                mAnimTargetChildNo = 1;
        } else {
                mAnimTargetChildNo = 0;
        }
}

void VerticalImageAutoScrollbox::setAllowImageUpscale(bool allowUpscaling)
{
        mAllowUpscaling = allowUpscaling;
}

void VerticalImageAutoScrollbox::addImage(ImageComponent *img)
{
        float posX;
        if (mCentered)
        {
                img->setOrigin(0.5f, 0.f);
                posX = getSize().x()/2.f;
        } else {
                img->setOrigin(0.f, 0.f);
                posX = 0.f;
        }
        if (getChildCount() == 0)
        {
                img->setPosition(posX, 0.f);
        } else {
                ImageComponent *last_img = static_cast<ImageComponent*>(getChild(getChildCount()-1));
                img->setPosition(posX, last_img->getPosition().y() + last_img->getSize().y() + mBorderSpace);
        }
        img->setResize(getSize().x(), 0.f, mAllowUpscaling);
        std::cout << __PRETTY_FUNCTION__ << " imgpos " << img->getPosition().x() << "," << img->getPosition().y() << std::endl;
        addChild(img);
}

void VerticalImageAutoScrollbox::setBorderSpace(float dist)
{
        mBorderSpace = dist;
}

void VerticalImageAutoScrollbox::reset()
{
        mScrollPos[1] = 0.f;
        setAutoScroll(mAutoScrollDelay, mAnimDuration);
}

void VerticalImageAutoScrollbox::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	Eigen::Vector2i clipPos((int)trans.translation().x(), (int)trans.translation().y());

	Eigen::Vector3f dimScaled = trans * Eigen::Vector3f(mSize.x(), mSize.y(), 0);
	Eigen::Vector2i clipDim((int)dimScaled.x() - trans.translation().x(), (int)dimScaled.y() - trans.translation().y());

	Renderer::pushClipRect(clipPos, clipDim);

	trans.translate(Eigen::Vector3f(-mScrollPos.x(), -mScrollPos.y(), 0.f));
	Renderer::setMatrix(trans);

	GuiComponent::renderChildren(trans);


        /*
        if (getSize().y() < getContentSize().y())
        {
                Eigen::Vector2f contentSize = getContentSize();
                while(mScrollPos.y() + getSize().y() > contentSize.y())
                {
                        trans.translate(Eigen::Vector3f(0.f, (float)(contentSize.y()+mBorderSpace), 0.f));
                        Renderer::setMatrix(trans);

                        GuiComponent::renderChildren(trans);
                        contentSize[1] += contentSize.y() + mBorderSpace;
                }
        }
        */

	Renderer::popClipRect();
}


void VerticalImageAutoScrollbox::update(int deltaTime)
{
        mAutoScrollTimer += deltaTime;
        if (mAutoScrollTimer > mAutoScrollDelay)
        {
                // animate
                if (mAutoScrollTimer > mAutoScrollDelay + mAnimDuration)
                {
                        // animation is over - scroll to targetPos
                        mScrollPos[1] = getAnimTargetPos(mAnimTargetChildNo);
                        // reset timer
                        mAutoScrollTimer = 0;
                        if (++mAnimTargetChildNo >= getChildCount())
                                mAnimTargetChildNo = 0;
                } else {
                        // perform animation step
                        float startPosY = getAnimTargetPos(mAnimTargetChildNo-1);
                        float deltaPosY = getAnimTargetPos(mAnimTargetChildNo) - startPosY;
                        float timeElapsed = static_cast<float>(mAutoScrollTimer - mAutoScrollDelay);
                        float duration = static_cast<float>(mAnimDuration);
                        // quadratic ease out by robert penner [http://robertpenner.com/easing/]
	                mScrollPos[1] = -deltaPosY *(timeElapsed/=duration)*(timeElapsed-2.f) + startPosY;
                }
        }
	GuiComponent::update(deltaTime);
}

float VerticalImageAutoScrollbox::getAnimTargetPos(unsigned int childNo) const
{
        return getChild(childNo%getChildCount())->getPosition().y();
        float offset = 0.f;
        int i = 0;
        if (getChildCount() == 0)
                return offset;
        offset = getChild(0)->getPosition().y();
        while (childNo >= getChildCount())
        {
                childNo -= getChildCount();
                offset += getContentSize().y() + mBorderSpace;
        }
        return offset + getChild(i)->getPosition().y();
        for (unsigned int i=0; i<childNo; ++i)
        {
                offset += getChild(i)->getSize().y() + mBorderSpace;
        }
        return offset;
}

Eigen::Vector2f VerticalImageAutoScrollbox::getContentSize() const
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
