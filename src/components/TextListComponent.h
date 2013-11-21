#ifndef _TEXTLISTCOMPONENT_H_
#define _TEXTLISTCOMPONENT_H_

#include "../Renderer.h"
#include "../resources/Font.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>
#include <memory>
#include "../Sound.h"
#include "../Log.h"
#include "../ThemeData.h"
#include <functional>

#define THEME_FONT "listFont"
#define THEME_SELECTOR_COLOR "listSelectorColor"
#define THEME_HIGHLIGHTED_COLOR "listSelectedColor"
#define THEME_SCROLL_SOUND "listScrollSound"
static const int THEME_COLOR_ID_COUNT = 2;
static const char* const THEME_ENTRY_COLOR[THEME_COLOR_ID_COUNT] = { "listPrimaryColor", "listSecondaryColor" };

//A graphical list. Supports multiple colors for rows and scrolling.
template <typename T>
class TextListComponent : public GuiComponent
{
public:
	TextListComponent(Window* window);
	virtual ~TextListComponent();

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	struct ListRow
	{
		std::string name;
		T object;
		unsigned int colorId;
		std::shared_ptr<TextCache> textCache;
	};

	void add(const std::string& name, const T& obj, unsigned int colorId);
	void remove(const T& obj);
	void clear();

	inline const std::string& getSelectedName() const { return mRowVector.at(mCursor).name; }
	inline T getSelected() const { return mRowVector.at(mCursor).object; }
	inline const std::vector<ListRow>& getList() const { return mRowVector; }

	void setCursor(const T& select);

	void stopScrolling();
	inline bool isScrolling() const { return mScrollDir != 0; }

	void setTheme(const std::shared_ptr<ThemeData>& theme);
	inline void setCentered(bool centered) { mCentered = centered; }

	enum CursorState {
		CURSOR_STOPPED,
		CURSOR_SCROLLING
	};

	inline void setCursorChangedCallback(const std::function<void(CursorState state)>& func) { mCursorChangedCallback = func; }

private:
	static const int MARQUEE_DELAY = 900;
	static const int MARQUEE_SPEED = 16;
	static const int MARQUEE_RATE = 3;

	static const int SCROLL_DELAY = 507;
	static const int SCROLL_TIME = 150;

	void scroll(); //helper method, scrolls in whatever direction scrollDir is
	void setScrollDir(int val); //helper method, set mScrollDir as well as reset marquee stuff
	void onCursorChanged(CursorState state);

	int mScrollDir, mScrollAccumulator;

	int mMarqueeOffset;
	int mMarqueeTime;

	std::shared_ptr<ThemeData> mTheme;
	bool mCentered;

	std::vector<ListRow> mRowVector;
	int mCursor;

	std::function<void(CursorState state)> mCursorChangedCallback;
};

template <typename T>
TextListComponent<T>::TextListComponent(Window* window) : 
	GuiComponent(window)
{
	mCursor = 0;
	mScrollDir = 0;
	mScrollAccumulator = 0;

	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;

	mCentered = true;

	mTheme = ThemeData::getDefault();
}

template <typename T>
TextListComponent<T>::~TextListComponent()
{
}

template <typename T>
void TextListComponent<T>::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	std::shared_ptr<Font> font = mTheme->getFont(THEME_FONT);

	const int cutoff = 0;
	const int entrySize = font->getHeight() + 5;

	int startEntry = 0;

	//number of entries that can fit on the screen simultaniously
	int screenCount = (int)mSize.y() / entrySize;
	
	if((int)mRowVector.size() >= screenCount)
	{
		startEntry = mCursor - (int)(screenCount * 0.5);
		if(startEntry < 0)
			startEntry = 0;
		if(startEntry >= (int)mRowVector.size() - screenCount)
			startEntry = mRowVector.size() - screenCount;
	}

	float y = (float)cutoff;

	if(mRowVector.size() == 0)
	{
		font->drawCenteredText("The list is empty.", 0, y, 0xFF0000FF);
		return;
	}

	int listCutoff = startEntry + screenCount;
	if(listCutoff > (int)mRowVector.size())
		listCutoff = mRowVector.size();

	Eigen::Vector3f dim(getSize().x(), getSize().y(), 0);
	dim = trans * dim - trans.translation();
	Renderer::pushClipRect(Eigen::Vector2i((int)trans.translation().x(), (int)trans.translation().y()), Eigen::Vector2i((int)dim.x(), (int)dim.y()));

	for(int i = startEntry; i < listCutoff; i++)
	{
		//draw selector bar
		if(mCursor == i)
		{
			Renderer::setMatrix(trans);
			Renderer::drawRect(0, (int)y, (int)getSize().x(), font->getHeight(), mTheme->getColor(THEME_SELECTOR_COLOR));
		}

		ListRow& row = mRowVector.at((unsigned int)i);

		float x = (float)(mCursor == i ? -mMarqueeOffset : 0);

		unsigned int color;
		if(mCursor == i && mTheme->getColor(THEME_HIGHLIGHTED_COLOR))
			color = mTheme->getColor(THEME_HIGHLIGHTED_COLOR);
		else
			color = mTheme->getColor(THEME_ENTRY_COLOR[row.colorId]);

		if(!row.textCache)
			row.textCache = std::unique_ptr<TextCache>(font->buildTextCache(row.name, 0, 0, 0x000000FF));

		row.textCache->setColor(color);

		Eigen::Vector3f offset(x, y, 0);

		if(mCentered)
			offset[0] += (mSize.x() - row.textCache->metrics.size.x()) / 2;

		Eigen::Affine3f drawTrans = trans;
		drawTrans.translate(offset);
		Renderer::setMatrix(drawTrans);

		font->renderTextCache(row.textCache.get());
		
		y += entrySize;
	}

	Renderer::popClipRect();

	GuiComponent::renderChildren(trans);
}

template <typename T>
bool TextListComponent<T>::input(InputConfig* config, Input input)
{
	if(mRowVector.size() > 0)
	{
		if(input.value != 0)
		{
			if(config->isMappedTo("down", input))
			{
				setScrollDir(1);
				scroll();
				return true;
			}

			if(config->isMappedTo("up", input))
			{
				setScrollDir(-1);
				scroll();
				return true;
			}
			if(config->isMappedTo("pagedown", input))
			{
				setScrollDir(10);
				scroll();
				return true;
			}

			if(config->isMappedTo("pageup", input))
			{
				setScrollDir(-10);
				scroll();
				return true;
			}
		}else{
			if(config->isMappedTo("down", input) || config->isMappedTo("up", input) || 
				config->isMappedTo("pagedown", input) || config->isMappedTo("pageup", input))
			{
				stopScrolling();
			}
		}
	}

	return GuiComponent::input(config, input);
}

template <typename T>
void TextListComponent<T>::setScrollDir(int val)
{
	mScrollDir = val;
	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;
	mScrollAccumulator = -SCROLL_DELAY;
}

template <typename T>
void TextListComponent<T>::stopScrolling()
{
	mScrollAccumulator = 0;
	mScrollDir = 0;
	onCursorChanged(CURSOR_STOPPED);
}

template <typename T>
void TextListComponent<T>::update(int deltaTime)
{
	if(mScrollDir != 0)
	{
		mScrollAccumulator += deltaTime;

		while(mScrollAccumulator >= SCROLL_TIME)
		{
			mScrollAccumulator -= SCROLL_TIME;
			scroll();
		}

	}else{
		//if we're not scrolling and this object's text goes outside our size, marquee it!
		std::string text = getSelectedName();

		Eigen::Vector2f textSize = mTheme->getFont(THEME_FONT)->sizeText(text);

		//it's long enough to marquee
		if(textSize.x() - mMarqueeOffset > getSize().x() - 12)
		{
			mMarqueeTime += deltaTime;
			while(mMarqueeTime > MARQUEE_SPEED)
			{
				mMarqueeOffset += MARQUEE_RATE;
				mMarqueeTime -= MARQUEE_SPEED;
			}
		}
	}

	GuiComponent::update(deltaTime);
}

template <typename T>
void TextListComponent<T>::scroll()
{
	mCursor += mScrollDir;

	if(mCursor < 0)
	{
		if(mScrollDir < -1)
			mCursor = 0;
		else
			mCursor += mRowVector.size();
	}
	if(mCursor >= (int)mRowVector.size())
	{
		if(mScrollDir > 1)
			mCursor = (int)mRowVector.size() - 1;
		else
			mCursor -= mRowVector.size();
	}

	onCursorChanged(CURSOR_SCROLLING);
	mTheme->playSound("scrollSound");
}

//list management stuff
template <typename T>
void TextListComponent<T>::add(const std::string& name, const T& obj, unsigned int color)
{
	if(color >= THEME_COLOR_ID_COUNT)
	{
		LOG(LogError) << "Invalid row color Id (" << color << ")";
		color = 0;
	}

	ListRow row = {name, obj, color};
	mRowVector.push_back(row);
}

template <typename T>
void TextListComponent<T>::remove(const T& obj)
{
	for(auto it = mRowVector.begin(); it != mRowVector.end(); it++)
	{
		if((*it).object == obj)
		{
			if(mCursor > 0 && it - mRowVector.begin() >= mCursor)
			{
				mCursor--;
				onCursorChanged(CURSOR_STOPPED);
			}

			mRowVector.erase(it);
			return;
		}
	}

	LOG(LogError) << "Tried to remove an object we couldn't find";
}

template <typename T>
void TextListComponent<T>::clear()
{
	mRowVector.clear();
	mCursor = 0;
	mScrollDir = 0;
	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;
	onCursorChanged(CURSOR_STOPPED);
}

template <typename T>
void TextListComponent<T>::setCursor(const T& obj)
{
	for(auto it = mRowVector.begin(); it != mRowVector.end(); it++)
	{
		if((*it).object == obj)
		{
			mCursor = it - mRowVector.begin();
			onCursorChanged(CURSOR_STOPPED);
			return;
		}
	}

	LOG(LogError) << "Tried to set cursor to object we couldn't find";
}

template <typename T>
void TextListComponent<T>::onCursorChanged(CursorState state)
{
	if(mCursorChangedCallback)
		mCursorChangedCallback(state);
}

template <typename T>
void TextListComponent<T>::setTheme(const std::shared_ptr<ThemeData>& theme)
{
	mTheme = theme;

	// invalidate text caches in case font changed
	for(auto it = mRowVector.begin(); it != mRowVector.end(); it++)
		it->textCache.reset();
}

#endif
