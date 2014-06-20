#pragma once

#include <string>
#include <vector>
#include <memory>
#include "GuiComponent.h"
#include "components/ImageComponent.h"
#include "resources/Font.h"
#include "Renderer.h"

enum CursorState
{
	CURSOR_STOPPED,
	CURSOR_SCROLLING
};

enum ListLoopType
{
	LIST_ALWAYS_LOOP,
	LIST_PAUSE_AT_END,
	LIST_NEVER_LOOP
};

struct ScrollTier
{
	int length; // how long we stay on this level before going to the next
	int scrollDelay; // how long between scrolls
};

struct ScrollTierList
{
	const int count;
	const ScrollTier* tiers;
};

// default scroll tiers
const ScrollTier QUICK_SCROLL_TIERS[] = {
	{500, 500},
	{5000, 114},
	{0, 8}
};
const ScrollTierList LIST_SCROLL_STYLE_QUICK = { 3, QUICK_SCROLL_TIERS };

const ScrollTier SLOW_SCROLL_TIERS[] = {
	{500, 500},
	{0, 150}
};
const ScrollTierList LIST_SCROLL_STYLE_SLOW = { 2, SLOW_SCROLL_TIERS };

template <typename EntryData, typename UserData>
class IList : public GuiComponent
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

	unsigned char mTitleOverlayOpacity;
	unsigned int mTitleOverlayColor;
	ImageComponent mGradient;
	std::shared_ptr<Font> mTitleOverlayFont;

	const ScrollTierList& mTierList;
	const ListLoopType mLoopType;

	std::vector<Entry> mEntries;
	
public:
	IList(Window* window, const ScrollTierList& tierList = LIST_SCROLL_STYLE_QUICK, const ListLoopType& loopType = LIST_PAUSE_AT_END) : GuiComponent(window), 
		mGradient(window), mTierList(tierList), mLoopType(loopType)
	{
		mCursor = 0;
		mScrollTier = 0;
		mScrollVelocity = 0;
		mScrollTierAccumulator = 0;
		mScrollCursorAccumulator = 0;
		
		mTitleOverlayOpacity = 0x00;
		mTitleOverlayColor = 0xFFFFFF00;
		mGradient.setResize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
		mGradient.setImage(":/scroll_gradient.png");
		mTitleOverlayFont = Font::get(FONT_SIZE_LARGE);
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
	void add(const Entry& e)
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
		if(mCursor > 0 && it - mEntries.begin() <= mCursor)
		{
			mCursor--;
			onCursorChanged(CURSOR_STOPPED);
		}

		mEntries.erase(it);
	}


	bool listInput(int velocity) // a velocity of 0 = stop scrolling
	{
		// generate an onCursorChanged event in the stopped state when the user lets go of the key
		if(velocity == 0 && mScrollVelocity != 0)
			onCursorChanged(CURSOR_STOPPED);

		mScrollVelocity = velocity;
		mScrollTier = 0;
		mScrollTierAccumulator = 0;
		mScrollCursorAccumulator = 0;

		int prevCursor = mCursor;
		scroll(mScrollVelocity);
		return (prevCursor != mCursor);
	}

	void listUpdate(int deltaTime)
	{
		// update the title overlay opacity
		const int dir = (mScrollTier >= mTierList.count - 1) ? 1 : -1; // fade in if scroll tier is >= 1, otherwise fade out
		int op = mTitleOverlayOpacity + deltaTime*dir; // we just do a 1-to-1 time -> opacity, no scaling
		if(op >= 255)
			mTitleOverlayOpacity = 255;
		else if(op <= 0)
			mTitleOverlayOpacity = 0;
		else
			mTitleOverlayOpacity = (unsigned char)op;

		if(mScrollVelocity == 0 || size() < 2)
			return;

		mScrollCursorAccumulator += deltaTime;
		mScrollTierAccumulator += deltaTime;

		// we delay scrolling until after scroll tier has updated so isScrolling() returns accurately during onCursorChanged callbacks
		// we don't just do scroll tier first because it would not catch the scrollDelay == tier length case
		int scrollCount = 0;
		while(mScrollCursorAccumulator >= mTierList.tiers[mScrollTier].scrollDelay)
		{
			mScrollCursorAccumulator -= mTierList.tiers[mScrollTier].scrollDelay;
			scrollCount++;
		}

		// are we ready to go even FASTER?
		while(mScrollTier < mTierList.count - 1 && mScrollTierAccumulator >= mTierList.tiers[mScrollTier].length)
		{
			mScrollTierAccumulator -= mTierList.tiers[mScrollTier].length;
			mScrollTier++;
		}

		// actually perform the scrolling
		for(int i = 0; i < scrollCount; i++)
			scroll(mScrollVelocity);
	}

	void listRenderTitleOverlay(const Eigen::Affine3f& trans)
	{
		if(size() == 0 || !mTitleOverlayFont || mTitleOverlayOpacity == 0)
			return;

		// we don't bother caching this because it's only two letters and will change pretty much every frame if we're scrolling
		const std::string text = getSelectedName().size() >= 2 ? getSelectedName().substr(0, 2) : "??";

		Eigen::Vector2f off = mTitleOverlayFont->sizeText(text);
		off[0] = (Renderer::getScreenWidth() - off.x()) * 0.5f;
		off[1] = (Renderer::getScreenHeight() - off.y()) * 0.5f;
		
		Eigen::Affine3f identTrans = Eigen::Affine3f::Identity();

		mGradient.setOpacity(mTitleOverlayOpacity);
		mGradient.render(identTrans);

		TextCache* cache = mTitleOverlayFont->buildTextCache(text, off.x(), off.y(), 0xFFFFFF00 | mTitleOverlayOpacity);
		mTitleOverlayFont->renderTextCache(cache); // relies on mGradient's render for Renderer::setMatrix()
		delete cache;
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
		if((mLoopType == LIST_PAUSE_AT_END && (mScrollTier > 0 || absAmt > 1)) ||
			mLoopType == LIST_NEVER_LOOP)
		{
			if(cursor < 0)
			{
				cursor = 0;
				mScrollVelocity = 0;
				mScrollTier = 0;
			}else if(cursor >= size())
			{
				cursor = size() - 1;
				mScrollVelocity = 0;
				mScrollTier = 0;
			}
		}else{
			while(cursor < 0)
				cursor += size();
			while(cursor >= size())
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
