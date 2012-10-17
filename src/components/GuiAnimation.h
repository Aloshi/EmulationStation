#ifndef _GUIANIMATION_H_
#define _GUIANIMATION_H_

#include "../GuiComponent.h"

class GuiAnimation : public GuiComponent
{
public:
	GuiAnimation();

	void move(int x, int y, int speed);
	void fadeIn(int time);
	void fadeOut(int time);

	void onTick(int deltaTime);
private:
	void moveChildren(int offsetx, int offsety);
	void setChildrenOpacity(unsigned char opacity);

	int mFadeRate;
	int mMoveX, mMoveY, mMoveSpeed;
};

#endif
