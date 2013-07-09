#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, std::shared_ptr<Font> font, Vector2i pos, Vector2u size) : GuiComponent(window), 
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

void TextComponent::setFont(std::shared_ptr<Font> font)
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

std::shared_ptr<Font> TextComponent::getFont() const
{
	if(mFont)
		return mFont;
	else
		return Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM);
}

void TextComponent::onRender()
{
	std::shared_ptr<Font> font = getFont();

	//Renderer::pushClipRect(getGlobalOffset(), getSize());

	font->drawWrappedText(mText, 0, 0, mSize.x, mColor >> 8 << 8  | getOpacity());

	//Renderer::popClipRect();

	GuiComponent::onRender();
}

void TextComponent::calculateExtent()
{
	std::shared_ptr<Font> font = getFont();

	if(mAutoCalcExtent.x)
		font->sizeText(mText, (int*)&mSize.x, (int*)&mSize.y);
	else
		if(mAutoCalcExtent.y)
			font->sizeWrappedText(mText, getSize().x, NULL, (int*)&mSize.y);
}
