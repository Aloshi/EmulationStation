#include "AnimationController.h"

AnimationController::AnimationController(Animation* anim, std::function<void()> finishedCallback, bool reverse)
	: mAnimation(anim), mFinishedCallback(finishedCallback), mReverse(reverse), mTime(0)
{
}

AnimationController::~AnimationController()
{
	if(mFinishedCallback)
		mFinishedCallback();

	delete mAnimation;
}

void AnimationController::update(int deltaTime)
{
	mTime += deltaTime;
	float t = (float)mTime / mAnimation->getDuration();

	if(t > 1.0f)
		t = 1.0f;
	else if(t < 0.0f)
		t = 0.0f;

	mAnimation->apply(mReverse ? 1.0f - t : t);

	if(t == 1.0f)
	{
		if(mFinishedCallback)
		{
			// in case mFinishedCallback causes us to be deleted, use a copy
			auto copy = mFinishedCallback;
			mFinishedCallback = nullptr;
			copy();
		}
	}
}
