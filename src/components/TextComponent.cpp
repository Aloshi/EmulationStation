#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"
#include "../ThemeData.h"

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

	unsigned char opacity = mColor & 0x000000FF;
	GuiComponent::setOpacity(opacity);

	onColorChanged();
}

void TextComponent::setOpacity(unsigned char opacity)
{
	mColor = (mColor & 0xFFFFFF00) | opacity;
	onColorChanged();

	GuiComponent::setOpacity(opacity);
}

unsigned char TextComponent::getOpacity() const
{
	return mColor & 0x000000FF;
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
		return Font::get(FONT_SIZE_MEDIUM);
}

void TextComponent::render(const Eigen::Affine3f& parentTrans)
{
	std::shared_ptr<Font> font = getFont();

	Eigen::Affine3f trans = parentTrans * getTransform();

	if(font && !mText.empty())
	{
		if(mCentered)
		{
			const Eigen::Vector2f& textSize = mTextCache->metrics.size;
			Eigen::Vector2f pos((getSize().x() - textSize.x()) / 2, 0);

			Eigen::Affine3f centeredTrans = trans;
			centeredTrans = centeredTrans.translate(Eigen::Vector3f(pos.x(), pos.y(), 0));
			Renderer::setMatrix(centeredTrans);
		}else{
			Renderer::setMatrix(trans);
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
	const bool wrap = (mSize.y() == 0 || (int)mSize.y() > f->getHeight());
	Eigen::Vector2f size = f->sizeText(mText);
	if(!wrap && mSize.x() && mText.size() && size.x() > mSize.x())
	{
		// abbreviate text
		const std::string abbrev = "...";
		Eigen::Vector2f abbrevSize = f->sizeText(abbrev);

		std::string text = mText;
		while(text.size() && size.x() + abbrevSize.x() > mSize.x())
		{
			text.erase(text.size() - 1, 1);
			size = f->sizeText(text);
		}

		text.append(abbrev);

		mTextCache = std::shared_ptr<TextCache>(f->buildTextCache(text, 0, 0, (mColor >> 8 << 8) | mOpacity));
	}else{
		mTextCache = std::shared_ptr<TextCache>(f->buildTextCache(f->wrapText(mText, mSize.x()), 0, 0, (mColor >> 8 << 8) | mOpacity));
	}
}

void TextComponent::onColorChanged()
{
	if(mTextCache)
	{
		mTextCache->setColor(mColor);
	}
}

void TextComponent::setValue(const std::string& value)
{
	setText(value);
}

std::string TextComponent::getValue() const
{
	return mText;
}

void TextComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "text");
	if(!elem)
		return;

	if(properties & COLOR && elem->has("color"))
		setColor(elem->get<unsigned int>("color"));

	if(properties & ALIGNMENT && elem->has("center"))
		setCentered(elem->get<bool>("center"));

	if(properties & TEXT && elem->has("text"))
		setText(elem->get<std::string>("text"));

	setFont(Font::getFromTheme(elem, properties, mFont));
}
