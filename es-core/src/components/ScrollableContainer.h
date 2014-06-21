#pragma once

#include "GuiComponent.h"

class ScrollableContainer : public GuiComponent
{
public:
	ScrollableContainer(Window* window);

	Eigen::Vector2f getScrollPos() const;
	void setScrollPos(const Eigen::Vector2f& pos);
	void setAutoScroll(bool autoScroll);
	void reset();

	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	Eigen::Vector2f getContentSize();

	Eigen::Vector2f mScrollPos;
	Eigen::Vector2f mScrollDir;
	int mAutoScrollDelay; // ms to wait before starting to autoscroll
	int mAutoScrollSpeed; // ms to wait before scrolling down by mScrollDir
	int mAutoScrollAccumulator;
	bool mAtEnd;
	int mAutoScrollResetAccumulator;
};
