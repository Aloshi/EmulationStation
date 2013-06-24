#include "SliderComponent.h"
#include <assert.h>
#include "../Renderer.h"

SliderComponent::SliderComponent(Window* window, float min, float max, float increment) : GuiComponent(window),
	mMin(min), mMax(max), mIncrement(increment), mMoveRate(0), mRepeatWaitTimer(0)
{
	assert((min - max) != 0);

	mValue = (max + min) / 2;

	//calculate move scale
	mMoveScale = ((max - min) * 0.0007f) / increment;

	setSize(Vector2u(128, 32));
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

		if(mRepeatWaitTimer < 450)
			mRepeatWaitTimer += deltaTime;
	}
	
	GuiComponent::update(deltaTime);
}

void SliderComponent::onRender()
{
	//render line
	const int lineWidth = 2;
	Renderer::drawRect(0, mSize.y / 2 - lineWidth / 2, mSize.x, lineWidth, 0x000000CC);

	//render left end
	const int capWidth = (int)(mSize.x * 0.03f);
	Renderer::drawRect(0, 0, capWidth, mSize.y, 0x000000CC);

	//render right end
	Renderer::drawRect(mSize.x - capWidth, 0, capWidth, mSize.y, 0x000000CC);

	//render our value
	const int lineLength = mSize.x - capWidth;
	Renderer::drawRect((int)(((mValue + mMin) / mMax) * lineLength), 0, capWidth, mSize.y, 0x0000FFFF);

	GuiComponent::onRender();
}

void SliderComponent::setSize(Vector2u size)
{
	mSize = size;
}

void SliderComponent::setValue(float value)
{
	mValue = value;
}

float SliderComponent::getValue()
{
	return mValue;
}
