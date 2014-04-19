#include "TextComponent.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"
#include "../ThemeData.h"
#include "../Util.h"
#include "../Settings.h"

TextComponent::TextComponent(Window* window) : GuiComponent(window), 
	mFont(Font::get(FONT_SIZE_MEDIUM)), mColor(0x000000FF), mAutoCalcExtent(true, true), mAlignment(ALIGN_LEFT)
{
}

TextComponent::TextComponent(Window* window, const std::string& text, const std::shared_ptr<Font>& font, unsigned int color, Alignment align,
	Eigen::Vector3f pos, Eigen::Vector2f size) : GuiComponent(window), 
	mFont(NULL), mColor(0x000000FF), mAutoCalcExtent(true, true), mAlignment(align)
{
	setFont(font);
	setColor(color);
	setText(text);
	setPosition(pos);
	setSize(size);
}

void TextComponent::onSizeChanged()
{
	mAutoCalcExtent << (getSize().x() == 0), (getSize().y() == 0);
	onTextChanged();
}

void TextComponent::setFont(const std::shared_ptr<Font>& font)
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

void TextComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	/*Eigen::Vector3f dim(mSize.x(), mSize.y(), 0);
	dim = trans * dim - trans.translation();
	Renderer::pushClipRect(Eigen::Vector2i((int)trans.translation().x(), (int)trans.translation().y()), 
		Eigen::Vector2i((int)(dim.x() + 0.5f), (int)(dim.y() + 0.5f)));
		*/

	if(mTextCache)
	{
		const Eigen::Vector2f& textSize = mTextCache->metrics.size;
		Eigen::Vector3f off(0, 0, 0);

		switch(mAlignment)
		{
		case ALIGN_LEFT:
			off << 0, (getSize().y() - textSize.y()) / 2, 0;
			break;

		case ALIGN_CENTER:
			off << (getSize().x() - textSize.x()) / 2, (getSize().y() - textSize.y()) / 2, 0;
			break;

		case ALIGN_RIGHT:
			off << (getSize().x() - textSize.x()), (getSize().y() - textSize.y()) / 2, 0;
			break;
		}

		if(Settings::getInstance()->getBool("DebugText"))
		{
			// draw the "textbox" area, what we are aligned within
			Renderer::setMatrix(trans);
			Renderer::drawRect(0.f, 0.f, mSize.x(), mSize.y(), 0xFF000033);
		}
		
		trans.translate(off);
		trans = roundMatrix(trans);
		Renderer::setMatrix(trans);

		// draw the text area, where the text actually is going
		if(Settings::getInstance()->getBool("DebugText"))
			Renderer::drawRect(0.0f, 0.0f, mTextCache->metrics.size.x(), mTextCache->metrics.size.y(), 0x00000033);

		mFont->renderTextCache(mTextCache.get());
	}

	//Renderer::popClipRect();
}

void TextComponent::calculateExtent()
{
	if(mAutoCalcExtent.x())
	{
		mSize = mFont->sizeText(mText);
	}else{
		if(mAutoCalcExtent.y())
		{
			mSize[1] = mFont->sizeWrappedText(mText, getSize().x()).y();
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

	if(properties & ALIGNMENT && elem->has("alignment"))
	{
		std::string str = elem->get<std::string>("alignment");
		if(str == "left")
			setAlignment(ALIGN_LEFT);
		else if(str == "center")
			setAlignment(ALIGN_CENTER);
		else if(str == "right")
			setAlignment(ALIGN_RIGHT);
		else
			LOG(LogError) << "Unknown text alignment string: " << str;
	}

	if(properties & TEXT && elem->has("text"))
		setText(elem->get<std::string>("text"));

	setFont(Font::getFromTheme(elem, properties, mFont));
}
