#include "ButtonComponent.h"
#include "../Renderer.h"
#include "../Window.h"

ButtonComponent::ButtonComponent(Window* window) : GuiComponent(window),
	mBox(window, ":/button.png"),
	mFocused(false), 
	mTextColorFocused(0x000000FF), mTextColorUnfocused(0x333333FF), mTextPulseTime(0)
{
	setSize(64, 48);
}

void ButtonComponent::onSizeChanged()
{
	mBox.setSize(mSize);
}

void ButtonComponent::setPressedFunc(std::function<void()> f)
{
	mPressedFunc = f;
}

bool ButtonComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		if(mPressedFunc)
			mPressedFunc();
		return true;
	}

	return GuiComponent::input(config, input);
}

void ButtonComponent::setText(const std::string& text, unsigned int focusedColor, unsigned int unfocusedColor)
{
	mText = text;
	mTextColorFocused = focusedColor;
	mTextColorUnfocused = unfocusedColor;

	std::shared_ptr<Font> f = getFont();
	mTextCache = std::unique_ptr<TextCache>(f->buildTextCache(mText, 0, 0, getCurTextColor()));

	setSize(mTextCache->metrics.size + Eigen::Vector2f(12, 12));
}

void ButtonComponent::onFocusGained()
{
	mFocused = true;
}

void ButtonComponent::onFocusLost()
{
	mFocused = false;
}

void ButtonComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	mBox.render(trans);

	if(mTextCache)
	{
		Eigen::Vector3f centerOffset((mSize.x() - mTextCache->metrics.size.x()) / 2, (mSize.y() - mTextCache->metrics.size.y()) / 2, 0);
		trans = trans.translate(centerOffset);

		Renderer::setMatrix(trans);
		mTextCache->setColor(getCurTextColor());
		getFont()->renderTextCache(mTextCache.get());
		trans = trans.translate(-centerOffset);
	}

	renderChildren(trans);
}

std::shared_ptr<Font> ButtonComponent::getFont()
{
	return Font::get(FONT_SIZE_SMALL);
}

unsigned int ButtonComponent::getCurTextColor() const
{
	if(!mFocused)
		return mTextColorUnfocused;
	else
		return mTextColorFocused;
}
