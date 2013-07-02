#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true), mAutoScrollDelay(0), mAutoScrollSpeed(0), mAutoScrollTimer(0)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, Font* font, Vector2i pos, Vector2u size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true), mAutoScrollDelay(0), mAutoScrollSpeed(0), mAutoScrollTimer(0)
{
	setText(text);
	setFont(font);
	setBox(pos, size);
}

void TextComponent::setBox(Vector2i pos, Vector2u size)
{
	setOffset(pos);
	setExtent(size);
}

void TextComponent::setExtent(Vector2u size)
{
	if(size == Vector2u(0, 0))
	{
		mAutoCalcExtent = true;
		calculateExtent();
	}else{
		mAutoCalcExtent = false;
		mSize = size;
	}
}

void TextComponent::setFont(Font* font)
{
	mFont = font;

	if(mAutoCalcExtent)
		calculateExtent();
}

void TextComponent::setColor(unsigned int color)
{
	mColor = color;
}

void TextComponent::setText(const std::string& text)
{
	mText = text;

	if(mAutoCalcExtent)
		calculateExtent();

	mScrollPos = Vector2d(0, 0);
	mScrollDir = Vector2d(0, 0);
	resetAutoScrollTimer();
}

Font* TextComponent::getFont() const
{
	return (mFont ? mFont : Renderer::getDefaultFont(Renderer::MEDIUM));;
}

void TextComponent::onRender()
{
	Font* font = getFont();
	if(font == NULL)
	{
		LOG(LogError) << "TextComponent can't get a valid font!";
		return;
	}

	Renderer::pushClipRect(getGlobalOffset(), getSize());

	Renderer::drawWrappedText(mText, (int)-mScrollPos.x, (int)-mScrollPos.y, mSize.x, mColor >> 8 << 8  | getOpacity(), font);

	Renderer::popClipRect();

	GuiComponent::onRender();
}

void TextComponent::calculateExtent()
{
	Font* font = getFont();
	if(font == NULL)
	{
		LOG(LogError) << "TextComponent can't get a valid font!";
		return;
	}

	font->sizeText(mText, (int*)&mSize.x, (int*)&mSize.y);
}

void TextComponent::setAutoScroll(int delay, double speed)
{
	mAutoScrollDelay = delay;
	mAutoScrollSpeed = speed;
	resetAutoScrollTimer();
}

void TextComponent::resetAutoScrollTimer()
{
	mAutoScrollTimer = 0;
}

Vector2d TextComponent::getScrollPos() const
{
	return mScrollPos;
}

void TextComponent::setScrollPos(const Vector2d& pos)
{
	mScrollPos = pos;
}

void TextComponent::update(int deltaTime)
{
	double scrollAmt = (double)deltaTime;

	if(mAutoScrollSpeed != 0)
	{
		mAutoScrollTimer += deltaTime;

		scrollAmt = (float)(mAutoScrollTimer - mAutoScrollDelay);

		if(scrollAmt > 0)
		{
			//scroll the amount of time left over from the delay
			mAutoScrollTimer = mAutoScrollDelay;

			//scale speed by our width! more text per line = slower scrolling
			const double widthMod = (680.0 / getSize().x);
			mScrollDir = Vector2d(0, mAutoScrollSpeed * widthMod);
		}else{
			//not enough to pass the delay, do nothing
			scrollAmt = 0;
		}
	}

	Vector2d scroll = mScrollDir * scrollAmt;
	mScrollPos += scroll;

	//clip scrolling within bounds
	if(mScrollPos.x < 0)
		mScrollPos.x = 0;
	if(mScrollPos.y < 0)
		mScrollPos.y = 0;

	Font* font = getFont();
	if(font != NULL)
	{
		Vector2i textSize;
		Renderer::sizeWrappedText(mText, getSize().x, getFont(), &textSize.x, &textSize.y);
		
		if(mScrollPos.x + getSize().x > textSize.x)
			mScrollPos.x = (double)textSize.x - getSize().x;
		if(mScrollPos.y + getSize().y > textSize.y)
			mScrollPos.y = (double)textSize.y - getSize().y;
	}

	GuiComponent::update(deltaTime);
}
