#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, std::shared_ptr<Font> font, Eigen::Vector3f pos, Eigen::Vector2f size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true)
{
	setText(text);
	setFont(font);
	setPosition(pos);
	setSize(size);
}

void TextComponent::onSizeChanged()
{
	mAutoCalcExtent << (getSize().x() == 0), (getSize().y() == 0);
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
	mOpacity = mColor & 0x000000FF;
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

void TextComponent::render(const Eigen::Affine3f& parentTrans)
{
	std::shared_ptr<Font> font = getFont();

	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);
	
	font->drawWrappedText(mText, Eigen::Vector2f(0, 0), getSize().x(), mColor >> 8 << 8  | getOpacity());

	GuiComponent::renderChildren(trans);
}

void TextComponent::calculateExtent()
{
	std::shared_ptr<Font> font = getFont();

	if(mAutoCalcExtent.x())
	{
		mSize = font->sizeText(mText);
	}else{
		if(mAutoCalcExtent.y())
		{
			mSize[1] = font->sizeWrappedText(mText, getSize().x()).y();
		}
	}
}
