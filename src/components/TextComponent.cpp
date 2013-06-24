#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, Font* font, Vector2i pos, Vector2u size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true)
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
}

void TextComponent::onRender()
{
	Font* font = (mFont ? mFont : Renderer::getDefaultFont(Renderer::MEDIUM));
	if(font == NULL)
	{
		LOG(LogError) << "TextComponent can't get a valid font!";
		return;
	}

	Renderer::pushClipRect(getGlobalOffset(), getSize());

	Renderer::drawWrappedText(mText, 0, 0, mSize.x, mColor, font);

	Renderer::popClipRect();

	GuiComponent::onRender();
}

void TextComponent::calculateExtent()
{
	Font* font = (mFont ? mFont : Renderer::getDefaultFont(Renderer::MEDIUM));
	if(font == NULL)
	{
		LOG(LogError) << "TextComponent can't get a valid font!";
		return;
	}

	font->sizeText(mText, (int*)&mSize.x, (int*)&mSize.y);
}
