#pragma once

#include "../GuiComponent.h"

class ScrollableContainer : public GuiComponent
{
public:
	ScrollableContainer(Window* window);

	void setSize(Vector2u size);

	Vector2d getScrollPos() const;
	void setScrollPos(const Vector2d& pos);
	void setAutoScroll(int delay, double speed); //Use 0 for speed to disable.
	void resetAutoScrollTimer();

	void update(int deltaTime) override;
	void render() override;

	//Vector2i getGlobalOffset() override;
private:
	Vector2i getContentSize();

	Vector2d mScrollPos;
	Vector2d mScrollDir;
	int mAutoScrollDelay;
	double mAutoScrollSpeed;
	int mAutoScrollTimer;
};
