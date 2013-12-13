#pragma once

#include <memory>
#include <functional>
#include "Animation.h"

class AnimationController
{
public:
	// FinishedCallback is guaranteed to be called exactly once, even if the animation does not finish normally.
	// Takes ownership of anim (will delete in destructor).
	AnimationController(Animation* anim, std::function<void()> finishedCallback = nullptr, bool reverse = false);
	virtual ~AnimationController();

	// Returns true if the animation is complete.
	bool update(int deltaTime);

private:
	Animation* mAnimation;
	std::function<void()> mFinishedCallback;
	bool mReverse;
	int mTime;
};
