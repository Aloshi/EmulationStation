#pragma once
#ifndef ES_APP_ANIMATIONS_MOVE_CAMERA_ANIMATION_H
#define ES_APP_ANIMATIONS_MOVE_CAMERA_ANIMATION_H

#include "animations/Animation.h"

class MoveCameraAnimation : public Animation
{
public:
	MoveCameraAnimation(Transform4x4f& camera, const Vector3f& target) : mCameraStart(camera), mTarget(target), mCameraOut(camera) { }

	int getDuration() const override { return 400; }

	void apply(float t) override
	{
		t -= 1;
		mCameraOut.translation() = -Vector3f().lerp(-mCameraStart.translation(), mTarget, t*t*t + 1 /*cubic ease out*/);
	}

private:
	Transform4x4f mCameraStart;
	Vector3f mTarget;

	Transform4x4f& mCameraOut;
};

#endif // ES_APP_ANIMATIONS_MOVE_CAMERA_ANIMATION_H
