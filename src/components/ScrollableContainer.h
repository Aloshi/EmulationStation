#pragma once

#include "../GuiComponent.h"

class ScrollableContainer : public GuiComponent
{
public:
	ScrollableContainer(Window* window);

	Eigen::Vector2d getScrollPos() const;
	void setScrollPos(const Eigen::Vector2d& pos);
	void setAutoScroll(int delay, double speed); //Use 0 for speed to disable.
        // don't stop scrolling at the end - instead circle forever forwards 
        // loopBorderDist is the distance between the last and the looped first child
        void setLoopForever(bool loopForever = true, float loopBorderDist = 0.f);
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
        bool mLoopForever;
        float mLoopBorderDist;
};
