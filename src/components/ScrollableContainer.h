#pragma once

#include "../GuiComponent.h"

class ScrollableContainer : public GuiComponent
{
public:
	ScrollableContainer(Window* window);

	Eigen::Vector2d getScrollPos() const;
	void setScrollPos(const Eigen::Vector2d& pos);
	void setAutoScroll(int delay, double speed); //Use 0 for speed to disable.
	void resetAutoScrollTimer();

	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	Eigen::Vector2f getContentSize();

	Eigen::Vector2d mScrollPos;
	Eigen::Vector2d mScrollDir;
	int mAutoScrollDelay;
	double mAutoScrollSpeed;
	int mAutoScrollTimer;
};
