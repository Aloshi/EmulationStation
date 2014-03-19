#include "SliderComponent.h"
#include <assert.h>
#include "../Renderer.h"
#include "../resources/Font.h"
#include "../Log.h"
#include "../Util.h"

SliderComponent::SliderComponent(Window* window, float min, float max, float increment, const std::string& suffix) : GuiComponent(window),
	mMin(min), mMax(max), mIncrement(increment), mMoveRate(0), mRepeatWaitTimer(0), mKnob(window), mSuffix(suffix)
{
	assert((min - max) != 0);

	mValue = (max + min) / 2;

	//calculate move scale
	mMoveScale = ((max - min) * 0.0007f) / increment;

	mKnob.setOrigin(0.5f, 0.5f);
	mKnob.setImage(":/slider_knob.png");
	
	setSize(196, 32);
}

bool SliderComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("left", input))
	{
		if(input.value)
			mMoveRate = -mIncrement;
		else
			mMoveRate = 0;

		//setting mRepeatWaitTimer to 0 will trigger an initial move in our update method
		mRepeatWaitTimer = 0;

		return true;
	}
	if(config->isMappedTo("right", input))
	{
		if(input.value)
			mMoveRate = mIncrement;
		else
			mMoveRate = 0;

		mRepeatWaitTimer = 0;

		return true;
	}

	return GuiComponent::input(config, input);
}

void SliderComponent::update(int deltaTime)
{
	if(mMoveRate != 0)
	{
		if(mRepeatWaitTimer == 0)
			mValue += mMoveRate;
		else if(mRepeatWaitTimer >= 450)
			mValue += mMoveRate * deltaTime * mMoveScale;

		if(mValue < mMin)
			mValue = mMin;
		if(mValue > mMax)
			mValue = mMax;

		onValueChanged();

		if(mRepeatWaitTimer < 450)
			mRepeatWaitTimer += deltaTime;
	}
	
	GuiComponent::update(deltaTime);
}

void SliderComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = roundMatrix(parentTrans * getTransform());
	Renderer::setMatrix(trans);

	// render suffix
	if(mValueCache)
		mFont->renderTextCache(mValueCache.get());

	float width = mSize.x() - mKnob.getSize().x() - (mValueCache ? mValueCache->metrics.size.x() + 4 : 0);

	//render line
	const float lineWidth = 2;
	Renderer::drawRect(mKnob.getSize().x() / 2, mSize.y() / 2 - lineWidth / 2, width, lineWidth, 0x777777FF);

	//render knob
	mKnob.render(trans);
	
	GuiComponent::renderChildren(trans);
}

void SliderComponent::setValue(float value)
{
	mValue = value;
	onValueChanged();
}

float SliderComponent::getValue()
{
	return mValue;
}

void SliderComponent::onSizeChanged()
{
	if(!mSuffix.empty())
		mFont = Font::get((int)(mSize.y() * 0.8f), FONT_PATH_LIGHT);
	
	onValueChanged();
}

void SliderComponent::onValueChanged()
{
	// update suffix textcache
	if(mFont)
	{
		std::stringstream ss;
		ss << std::fixed;
		ss.precision(0);
		ss << mValue;
		ss << mSuffix;
		const std::string val = ss.str();

		ss.str("");
		ss.clear();
		ss << std::fixed;
		ss.precision(0);
		ss << mMax;
		ss << mSuffix;
		const std::string max = ss.str();

		Eigen::Vector2f textSize = mFont->sizeText(max);
		mValueCache = std::shared_ptr<TextCache>(mFont->buildTextCache(val, mSize.x() - textSize.x(), (mSize.y() - textSize.y()) / 2, 0x777777FF));
		mValueCache->metrics.size[0] = textSize.x(); // fudge the width
	}

	// update knob position/size
	if(mKnob.getTextureSize().y() > mSize.y()) // only downscale
		mKnob.setResize(0, mSize.y());
	else
		mKnob.setResize(0, 0);

	float lineLength = mSize.x() - mKnob.getSize().x() - (mValueCache ? mValueCache->metrics.size.x() + 4 : 0);
	mKnob.setPosition(((mValue + mMin) / mMax) * lineLength + mKnob.getSize().x()/2, mSize.y() / 2);
}

std::vector<HelpPrompt> SliderComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", "adjust"));
	return prompts;
}
