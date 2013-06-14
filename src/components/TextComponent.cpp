#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), mFont(NULL), mColor(0x000000FF)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, Font* font, Vector2i pos, Vector2u size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF)
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
}

void TextComponent::setFont(Font* font)
{
	mFont = font;
}

void TextComponent::setColor(unsigned int color)
{
	mColor = color;
}

void TextComponent::setText(const std::string& text)
{
	mText = text;
}

void TextComponent::onRender()
{
	Font* font = (mFont ? mFont : Renderer::getDefaultFont(Renderer::MEDIUM));
	if(font == NULL)
	{
		LOG(LogError) << "TextComponent can't get a valid font!";
		return;
	}

	Renderer::pushClipRect(getOffset(), getSize());

	Renderer::drawWrappedText(mText, 0, 0, mSize.x, mColor, font);

	Renderer::popClipRect();

	GuiComponent::onRender();
}
