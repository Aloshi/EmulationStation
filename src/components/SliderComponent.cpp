#include "SliderComponent.h"
#include <assert.h>
#include "../Renderer.h"
#include "../resources/Font.h"
#include "../Log.h"

SliderComponent::SliderComponent(Window* window, float min, float max, float increment, const std::string& suffix) : GuiComponent(window),
	mMin(min), mMax(max), mIncrement(increment), mMoveRate(0), mRepeatWaitTimer(0), mSuffix(suffix)
{
	assert((min - max) != 0);

	mValue = (max + min) / 2;

	//calculate move scale
	mMoveScale = ((max - min) * 0.0007f) / increment;

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
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	float width = mSize.x() - (mValueCache ? mValueCache->metrics.size.x() + 4 : 0);

	//render line
	const int lineWidth = 2;
	Renderer::drawRect(0, (int)mSize.y() / 2 - lineWidth / 2, (int)width, lineWidth, 0x000000CC);

	//render left end
	const int capWidth = (int)(mSize.x() * 0.03f);
	Renderer::drawRect(0, 0, capWidth, (int)mSize.y(), 0x000000CC);

	//render right end
	Renderer::drawRect((int)width - capWidth, 0, capWidth, (int)mSize.y(), 0x000000CC);

	//render our value
	const int lineLength = (int)width - capWidth;
	Renderer::drawRect((int)(((mValue + mMin) / mMax) * lineLength), 0, capWidth, (int)mSize.y(), 0x0000FFFF);

	// suffix
	if(mValueCache)
		mFont->renderTextCache(mValueCache.get());
	
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
	{
		mFont = Font::get((int)(mSize.y() * 0.7f));
		onValueChanged();
	}
}

void SliderComponent::onValueChanged()
{
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

		float w = mFont->sizeText(max).x();
		mValueCache = std::shared_ptr<TextCache>(mFont->buildTextCache(val, mSize.x() - w, 0, 0x000000FF));
		mValueCache->metrics.size[0] = w; // fudge the width
	}
}

std::vector<HelpPrompt> SliderComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", "adjust"));
	return prompts;
}
