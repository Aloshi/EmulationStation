#pragma once

class IList
{
public:
	IList();

	bool isScrolling() const;
	
protected:
	void listInput(int velocity); // a velocity of 0 = stop scrolling
	void listUpdate(int deltaTime);

	virtual int getCursorIndex() = 0;
	virtual void setCursorIndex(int index) = 0; // (index >= 0 && index < getLength()) is guaranteed to be true
	virtual int getLength() = 0;

	void scroll(int amt);
	virtual void onScroll(int amt) {};

private:
	struct ScrollTier
	{
		int length; // how long we stay on this level before going to the next
		int scrollDelay; // how long between scrolls
	};

	static const int SCROLL_SPEED_COUNT = 3;
	static const ScrollTier SCROLL_SPEED[SCROLL_SPEED_COUNT];

	int mScrollTier;
	int mScrollVelocity;

	int mScrollTierAccumulator;
	int mScrollCursorAccumulator;
};
