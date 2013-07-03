#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, Font* font, Vector2i pos, Vector2u size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true)
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
	mSize = size;
	mAutoCalcExtent = Vector2<bool>(size.x == 0, size.y == 0);
	calculateExtent();
}

void TextComponent::setFont(Font* font)
{
	mFont = font;

	calculateExtent();
}

void TextComponent::setColor(unsigned int color)
{
	mColor = color;
}

void TextComponent::setText(const std::string& text)
{
	mText = text;

	calculateExtent();
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

	//Renderer::pushClipRect(getGlobalOffset(), getSize());

	Renderer::drawWrappedText(mText, 0, 0, mSize.x, mColor >> 8 << 8  | getOpacity(), font);

	//Renderer::popClipRect();

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

	if(mAutoCalcExtent.x)
		font->sizeText(mText, (int*)&mSize.x, (int*)&mSize.y);
	else
		if(mAutoCalcExtent.y)
			Renderer::sizeWrappedText(mText, getSize().x, mFont, NULL, (int*)&mSize.y);
}
