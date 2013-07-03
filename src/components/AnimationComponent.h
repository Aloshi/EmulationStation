#ifndef _ANIMATIONCOMPONENT_H_
#define _ANIMATIONCOMPONENT_H_

#include "../GuiComponent.h"
#include "ImageComponent.h"
#include <vector>

#define ANIMATION_TICK_SPEED 16

class AnimationComponent
{
public:
	AnimationComponent();

	void move(int x, int y, int speed);
	void fadeIn(int time);
	void fadeOut(int time);

	void update(int deltaTime);

	void addChild(GuiComponent* gui);

private:
	unsigned char mOpacity;

	std::vector<GuiComponent*> mChildren;

	void moveChildren(int offsetx, int offsety);
	void setChildrenOpacity(unsigned char opacity);

	int mFadeRate;
	int mMoveX, mMoveY, mMoveSpeed;

	int mAccumulator;
};

#endif
