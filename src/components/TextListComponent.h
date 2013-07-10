#ifndef _TEXTLISTCOMPONENT_H_
#define _TEXTLISTCOMPONENT_H_

#include "../Renderer.h"
#include "../Font.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>
#include <memory>
#include "../Sound.h"
#include "../Log.h"

#define MARQUEE_DELAY 900
#define MARQUEE_SPEED 16
#define MARQUEE_RATE 3

//A graphical list. Supports multiple colors for rows and scrolling.
template <typename T>
class TextListComponent : public GuiComponent
{
public:
	TextListComponent(Window* window, float offsetX, float offsetY, std::shared_ptr<Font> font);
	virtual ~TextListComponent();

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void onPositionChanged() override;

	void addObject(std::string name, T obj, unsigned int color = 0xFF0000);
	void clear();

	std::string getSelectedName();
	T getSelectedObject();
	int getSelection();
	void stopScrolling();
	bool isScrolling();

	void setSelectorColor(unsigned int selectorColor);
	void setSelectedTextColor(unsigned int selectedColor);
	void setCentered(bool centered);
	void setScrollSound(std::shared_ptr<Sound> & sound);
	void setTextOffsetX(int textoffsetx);

	int getObjectCount();
	T getObject(int i);
	void setSelection(int i);

	void setFont(std::shared_ptr<Font> f);

private:
	static const int SCROLLDELAY = 507;
	static const int SCROLLTIME = 200;

	void scroll(); //helper method, scrolls in whatever direction scrollDir is
	void setScrollDir(int val); //helper method, set mScrollDir as well as reset marquee stuff

	int mScrollDir, mScrollAccumulator;
	bool mScrolling;

	int mMarqueeOffset;
	int mMarqueeTime;

	std::shared_ptr<Font> mFont;
	unsigned int mSelectorColor, mSelectedTextColorOverride;
	bool mDrawCentered;

	int mTextOffsetX;

	struct ListRow
	{
		std::string name;
		T object;
		unsigned int color;
	};

	std::vector<ListRow> mRowVector;
	int mSelection;
	std::shared_ptr<Sound> mScrollSound;
};

template <typename T>
TextListComponent<T>::TextListComponent(Window* window, float offsetX, float offsetY, std::shared_ptr<Font> font) : GuiComponent(window)
{
	mSelection = 0;
	mScrollDir = 0;
	mScrolling = 0;
	mScrollAccumulator = 0;

	setPosition(offsetX, offsetY);
	
	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;
	mTextOffsetX = 0;

	mFont = font;
	mSelectorColor = 0x000000FF;
	mSelectedTextColorOverride = 0;
	mScrollSound = NULL;
	mDrawCentered = true;
}

template <typename T>
void TextListComponent<T>::onPositionChanged()
{
	setSize(Renderer::getScreenWidth() - getPosition().x(), Renderer::getScreenHeight() - getPosition().y());
}

template <typename T>
TextListComponent<T>::~TextListComponent()
{
}

template <typename T>
void TextListComponent<T>::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	const int cutoff = 0;
	const int entrySize = mFont->getHeight() + 5;

	int startEntry = 0;

	//number of entries that can fit on the screen simultaniously
	int screenCount = (Renderer::getScreenHeight() - cutoff) / entrySize;
	screenCount -= 1;

	if((int)mRowVector.size() >= screenCount)
	{
		startEntry = mSelection - (int)(screenCount * 0.5);
		if(startEntry < 0)
			startEntry = 0;
		if(startEntry >= (int)mRowVector.size() - screenCount)
			startEntry = mRowVector.size() - screenCount;
	}

	float y = (float)cutoff;

	if(mRowVector.size() == 0)
	{
		mFont->drawCenteredText("The list is empty.", 0, y, 0xFF0000FF);
		return;
	}

	int listCutoff = startEntry + screenCount;
	if(listCutoff > (int)mRowVector.size())
		listCutoff = mRowVector.size();

	Renderer::pushClipRect(Eigen::Vector2i((int)trans.translation().x(), (int)trans.translation().y()), Eigen::Vector2i((int)getSize().x(), (int)getSize().y()));
	//Renderer::pushClipRect(getGlobalOffset(), getSize());

	for(int i = startEntry; i < listCutoff; i++)
	{
		//draw selector bar
		if(mSelection == i)
		{
			Renderer::drawRect(0, (int)y, (int)getSize().x(), mFont->getHeight(), mSelectorColor);
		}

		ListRow row = mRowVector.at((unsigned int)i);

		float x = (float)mTextOffsetX - (mSelection == i ? mMarqueeOffset : 0);
		unsigned int color = (mSelection == i && mSelectedTextColorOverride != 0) ? mSelectedTextColorOverride : row.color;

		if(mDrawCentered)
			mFont->drawCenteredText(row.name, x, y, color);
		else
			mFont->drawText(row.name, Eigen::Vector2f(x, y), color);

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
			if(config->isMappedTo("down", input) || config->isMappedTo("up", input) || config->isMappedTo("pagedown", input) || config->isMappedTo("pageup", input))
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
}

template <typename T>
void TextListComponent<T>::stopScrolling()
{
	mScrollAccumulator = 0;
	mScrolling = false;
	mScrollDir = 0;
}

template <typename T>
void TextListComponent<T>::update(int deltaTime)
{
	if(mScrollDir != 0)
	{
		mScrollAccumulator += deltaTime;

		if(!mScrolling)
		{
			if(mScrollAccumulator >= SCROLLDELAY)
			{
				mScrollAccumulator = SCROLLTIME;
				mScrolling = true;
			}
		}

		if(mScrolling)
		{
			mScrollAccumulator += deltaTime;

			while(mScrollAccumulator >= SCROLLTIME)
			{
				mScrollAccumulator -= SCROLLTIME;

				scroll();
			}
		}
	}else{
		//if we're not scrolling and this object's text goes outside our size, marquee it!
		std::string text = getSelectedName();

		Eigen::Vector2f textSize = mFont->sizeText(text);

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
	mSelection += mScrollDir;

	if(mSelection < 0)
	{
		if(mScrollDir < -1)
			mSelection = 0;
		else
			mSelection += mRowVector.size();
	}
	if(mSelection >= (int)mRowVector.size())
	{
		if(mScrollDir > 1)
			mSelection = (int)mRowVector.size() - 1;
		else
			mSelection -= mRowVector.size();
	}

	if(mScrollSound)
		mScrollSound->play();
}

//list management stuff
template <typename T>
void TextListComponent<T>::addObject(std::string name, T obj, unsigned int color)
{
	ListRow row = {name, obj, color};
	mRowVector.push_back(row);
}

template <typename T>
void TextListComponent<T>::clear()
{
	mRowVector.clear();
	mSelection = 0;
	mMarqueeOffset = 0;
	mMarqueeTime = -MARQUEE_DELAY;
}

template <typename T>
std::string TextListComponent<T>::getSelectedName()
{
	if((int)mRowVector.size() > mSelection)
		return mRowVector.at(mSelection).name;
	else
		return "";
}

template <typename T>
T TextListComponent<T>::getSelectedObject()
{
	if((int)mRowVector.size() > mSelection)
		return mRowVector.at(mSelection).object;
	else
		return NULL;
}

template <typename T>
int TextListComponent<T>::getSelection()
{
	return mSelection;
}

template <typename T>
bool TextListComponent<T>::isScrolling()
{
	return mScrollDir != 0;
}

template <typename T>
void TextListComponent<T>::setSelectorColor(unsigned int selectorColor)
{
	mSelectorColor = selectorColor;
}

template <typename T>
void TextListComponent<T>::setSelectedTextColor(unsigned int selectedColor)
{
	mSelectedTextColorOverride = selectedColor;
}

template<typename T>
void TextListComponent<T>::setCentered(bool centered)
{
	mDrawCentered = centered;
}

template<typename T>
void TextListComponent<T>::setTextOffsetX(int textoffsetx)
{
	mTextOffsetX = textoffsetx;
}

template <typename T>
int TextListComponent<T>::getObjectCount()
{
	return mRowVector.size();
}

template <typename T>
T TextListComponent<T>::getObject(int i)
{
	return mRowVector.at(i).object;
}

template <typename T>
void TextListComponent<T>::setSelection(int i)
{
	mSelection = i;
}

template <typename T>
void TextListComponent<T>::setScrollSound(std::shared_ptr<Sound> & sound)
{
	mScrollSound = sound;
}

template <typename T>
void TextListComponent<T>::setFont(std::shared_ptr<Font> font)
{
	mFont = font;
}

#endif
