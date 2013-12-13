#pragma once

#include "Animation.h"

class LambdaAnimation : public Animation
{
public:
	LambdaAnimation(const std::function<void(float t)>& func, int duration) : mFunction(func), mDuration(duration) {}

	int getDuration() const override { return mDuration; }

	void apply(float t) override
	{
		mFunction(t);
	}

private:
	std::function<void(float t)> mFunction;
	int mDuration;
};
