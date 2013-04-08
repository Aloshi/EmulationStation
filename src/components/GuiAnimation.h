#ifndef _GUIANIMATION_H_
#define _GUIANIMATION_H_

#include "../Gui.h"
#include "GuiImage.h"
#include <vector>

class GuiAnimation
{
public:
	GuiAnimation();

	void move(int x, int y, int speed);
	void fadeIn(int time);
	void fadeOut(int time);

	void update(int deltaTime);

	void addChild(GuiImage* gui);

private:
	unsigned char mOpacity;

	std::vector<GuiImage*> mChildren;

	void moveChildren(int offsetx, int offsety);
	void setChildrenOpacity(unsigned char opacity);

	int mFadeRate;
	int mMoveX, mMoveY, mMoveSpeed;
};

#endif
