#pragma once

#include <string>
#include <vector>

enum CursorState
{
	CURSOR_STOPPED,
	CURSOR_SCROLLING
};

struct ScrollTier
{
	int length; // how long we stay on this level before going to the next
	int scrollDelay; // how long between scrolls
};

const int SCROLL_SPEED_COUNT = 3;
const ScrollTier SCROLL_SPEED[SCROLL_SPEED_COUNT] = {
	{500, 500},
	{2600, 150},
	{0, 100}
};

template <typename EntryData, typename UserData>
class IList
{
public:
	struct Entry
	{
		std::string name;
		UserData object;
		EntryData data;
	};

protected:
	int mCursor;

	int mScrollTier;
	int mScrollVelocity;

	int mScrollTierAccumulator;
	int mScrollCursorAccumulator;

	std::vector<Entry> mEntries;
	
public:
	IList()
	{
		mCursor = 0;
		mScrollTier = 0;
		mScrollVelocity = 0;
		mScrollTierAccumulator = 0;
		mScrollCursorAccumulator = 0;
	}

	bool isScrolling() const
	{
		return (mScrollVelocity != 0 && mScrollTier > 0);
	}

	void stopScrolling()
	{
		listInput(0);
		onCursorChanged(CURSOR_STOPPED);
	}

	void clear()
	{
		mEntries.clear();
		mCursor = 0;
		listInput(0);
		onCursorChanged(CURSOR_STOPPED);
	}

	inline const std::string& getSelectedName()
	{
		assert(size() > 0);
		return mEntries.at(mCursor).name;
	}

	inline const UserData& getSelected() const
	{
		assert(size() > 0);
		return mEntries.at(mCursor).object;
	}

	void setCursor(typename std::vector<Entry>::iterator& it)
	{
		assert(it != mEntries.end());
		mCursor = it - mEntries.begin();
		onCursorChanged(CURSOR_STOPPED);
	}

	// returns true if successful (select is in our list), false if not
	bool setCursor(const UserData& obj)
	{
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
		{
			if((*it).object == obj)
			{
				mCursor = it - mEntries.begin();
				onCursorChanged(CURSOR_STOPPED);
				return true;
			}
		}

		return false;
	}
	
	// entry management
	void add(Entry e)
	{
		mEntries.push_back(e);
	}

	bool remove(const UserData& obj)
	{
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
		{
			if((*it).object == obj)
			{
				remove(it);
				return true;
			}
		}

		return false;
	}

	inline int size() const { return mEntries.size(); }

protected:
	void remove(typename std::vector<Entry>::iterator& it)
	{
		if(getCursorIndex() > 0 && it - mEntries.begin() <= getCursorIndex())
		{
			setCursorIndex(mCursor - 1);
			onCursorChanged(CURSOR_STOPPED);
		}

		mEntries.erase(it);
	}


	void listInput(int velocity) // a velocity of 0 = stop scrolling
	{
		mScrollVelocity = velocity;
		mScrollTier = 0;
		mScrollTierAccumulator = 0;
		mScrollCursorAccumulator = 0;
		scroll(mScrollVelocity);
	}

	void listUpdate(int deltaTime)
	{
		if(mScrollVelocity == 0 || size() < 2)
			return;

		mScrollCursorAccumulator += deltaTime;
		mScrollTierAccumulator += deltaTime;

		int scrollCount = 0;
		while(mScrollCursorAccumulator >= SCROLL_SPEED[mScrollTier].scrollDelay)
		{
			mScrollCursorAccumulator -= SCROLL_SPEED[mScrollTier].scrollDelay;
			scrollCount++;
		}

		// are we ready to go even FASTER?
		while(mScrollTier < SCROLL_SPEED_COUNT - 1 && mScrollTierAccumulator >= SCROLL_SPEED[mScrollTier].length)
		{
			mScrollTierAccumulator -= SCROLL_SPEED[mScrollTier].length;
			mScrollTier++;
		}

		for(int i = 0; i < scrollCount; i++)
			scroll(mScrollVelocity);
	}

	void scroll(int amt)
	{
		if(mScrollVelocity == 0 || size() < 2)
			return;

		int cursor = mCursor + amt;
		int absAmt = amt < 0 ? -amt : amt;

		// stop at the end if we've been holding down the button for a long time or
		// we're scrolling faster than one item at a time (e.g. page up/down)
		// otherwise, loop around
		if(mScrollTier > 0 || absAmt > 1)
		{
			if(cursor < 0)
				cursor = 0;
			else if(cursor >= size())
				cursor = size() - 1;
		}else{
			if(cursor < 0)
				cursor += size();
			else if(cursor >= size())
				cursor -= size();
		}

		if(cursor != mCursor)
			onScroll(absAmt);

		mCursor = cursor;
		onCursorChanged((mScrollTier > 0) ? CURSOR_SCROLLING : CURSOR_STOPPED);
	}

	virtual void onCursorChanged(const CursorState& state) {}
	virtual void onScroll(int amt) {}
};
