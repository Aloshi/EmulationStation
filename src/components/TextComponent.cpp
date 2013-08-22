#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true), mCentered(false)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, std::shared_ptr<Font> font, Eigen::Vector3f pos, Eigen::Vector2f size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true), mCentered(false)
{
	setText(text);
	setFont(font);
	setPosition(pos);
	setSize(size);
}

void TextComponent::onSizeChanged()
{
	mAutoCalcExtent << (getSize().x() == 0), (getSize().y() == 0);
	onTextChanged();
}

void TextComponent::setFont(std::shared_ptr<Font> font)
{
	mFont = font;
	onTextChanged();
}

void TextComponent::setColor(unsigned int color)
{
	mColor = color;
	mOpacity = mColor & 0x000000FF;
	onTextChanged();
}

void TextComponent::setText(const std::string& text)
{
	mText = text;
	onTextChanged();
}

void TextComponent::setCentered(bool center)
{
	mCentered = center;
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

	if(font && !mText.empty())
	{
		Renderer::setMatrix(trans);

		if(mCentered)
		{
			Eigen::Vector2f textSize = font->sizeWrappedText(mText, getSize().x());
			Eigen::Vector2f pos((getSize().x() - textSize.x()) / 2, 0);

			Eigen::Affine3f centeredTrans = trans;
			centeredTrans = centeredTrans.translate(Eigen::Vector3f(pos.x(), pos.y(), 0));
			Renderer::setMatrix(centeredTrans);
		}

		font->renderTextCache(mTextCache.get());
	}

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

void TextComponent::onTextChanged()
{
	calculateExtent();

	std::shared_ptr<Font> f = getFont();
	mTextCache = std::unique_ptr<TextCache>(f->buildTextCache(f->wrapText(mText, mSize.x()), 0, 0, (mColor >> 8 << 8) | mOpacity));
}

void TextComponent::setValue(const std::string& value)
{
	setText(value);
}

std::string TextComponent::getValue() const
{
	return mText;
}
