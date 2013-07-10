#include "AnimationComponent.h"

AnimationComponent::AnimationComponent()
{
	mMoveX = 0;
	mMoveY = 0;
	mMoveSpeed = 0;
	mFadeRate = 0;
	mOpacity = 0;
	mAccumulator = 0;
}

void AnimationComponent::move(int x, int y, int speed)
{
	mMoveX = x;
	mMoveY = y;
	mMoveSpeed = speed;
}

void AnimationComponent::fadeIn(int time)
{
	mOpacity = 0;
	setChildrenOpacity(0);

	mFadeRate = time;
}

void AnimationComponent::fadeOut(int time)
{
	mOpacity = 255;
	setChildrenOpacity(255);

	mFadeRate = -time;
}

//this should really be fixed at the system loop level...
void AnimationComponent::update(int deltaTime)
{
	mAccumulator += deltaTime;
	while(mAccumulator >= ANIMATION_TICK_SPEED)
	{
		mAccumulator -= ANIMATION_TICK_SPEED;

		if(mMoveX != 0 || mMoveY != 0)
		{
			Eigen::Vector2i offset(mMoveX, mMoveY);
			if(abs(offset.x()) > mMoveSpeed)
				offset.x() = mMoveSpeed * (offset.x() > 0 ? 1 : -1);
			if(abs(offset.y()) > mMoveSpeed)
				offset.y() = mMoveSpeed * (offset.y() > 0 ? 1 : -1);

			moveChildren(offset.x(), offset.y());

			mMoveX -= offset.x();
			mMoveY -= offset.y();
		}

		if(mFadeRate != 0)
		{
			int opacity = (int)mOpacity + mFadeRate;
			if(opacity > 255)
			{
				mFadeRate = 0;
				opacity = 255;
			}

			if(opacity < 0)
			{
				mFadeRate = 0;
				opacity = 0;
			}

			mOpacity = (unsigned char)opacity;
			setChildrenOpacity((unsigned char)opacity);
		}
	}
}

void AnimationComponent::addChild(GuiComponent* gui)
{
	mChildren.push_back(gui);
}

void AnimationComponent::moveChildren(int offsetx, int offsety)
{
	Eigen::Vector3f move((float)offsetx, (float)offsety, 0);
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		GuiComponent* comp = mChildren.at(i);
		comp->setPosition(comp->getPosition() + move);
	}
}

void AnimationComponent::setChildrenOpacity(unsigned char opacity)
{
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->setOpacity(opacity);
	}
}
