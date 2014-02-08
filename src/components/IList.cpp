#include "IList.h"

const IList::ScrollTier IList::SCROLL_SPEED[IList::SCROLL_SPEED_COUNT] = {
	{500, 500},
	{2600, 150},
	{0, 100}
};

IList::IList()
{
	mScrollTier = 0;
	mScrollVelocity = 0;
	mScrollTierAccumulator = 0;
	mScrollCursorAccumulator = 0;
}

void IList::listInput(int velocity)
{
	mScrollVelocity = velocity;
	mScrollTier = 0;
	mScrollTierAccumulator = 0;
	mScrollCursorAccumulator = 0;
	scroll(mScrollVelocity);
}

void IList::listUpdate(int deltaTime)
{
	if(mScrollVelocity == 0 || getLength() < 2)
		return;

	mScrollCursorAccumulator += deltaTime;
	mScrollTierAccumulator += deltaTime;

	while(mScrollCursorAccumulator >= SCROLL_SPEED[mScrollTier].scrollDelay)
	{
		mScrollCursorAccumulator -= SCROLL_SPEED[mScrollTier].scrollDelay;
		scroll(mScrollVelocity);
	}

	// are we ready to go even FASTER?
	while(mScrollTier < SCROLL_SPEED_COUNT - 1 && mScrollTierAccumulator >= SCROLL_SPEED[mScrollTier].length)
	{
		mScrollTierAccumulator -= SCROLL_SPEED[mScrollTier].length;
		mScrollTier++;
	}
}

void IList::scroll(int amt)
{
	if(mScrollVelocity == 0 || getLength() < 2)
		return;

	int cursor = getCursorIndex() + amt;
	int absAmt = amt < 0 ? -amt : amt;

	// stop at the end if we've been holding down the button for a long time or
	// we're scrolling faster than one item at a time (e.g. page up/down)
	// otherwise, loop around
	if(mScrollTier > 0 || absAmt > 1)
	{
		if(cursor < 0)
			cursor = 0;
		else if(cursor >= getLength())
			cursor = getLength() - 1;
	}else{
		if(cursor < 0)
			cursor += getLength();
		else if(cursor >= getLength())
			cursor -= getLength();
	}

	if(cursor != getCursorIndex())
		onScroll(absAmt);

	setCursorIndex(cursor);
}

bool IList::isScrolling() const
{
	return (mScrollVelocity != 0 && mScrollTier > 0);
}
