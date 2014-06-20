#pragma once

#include "animations/Animation.h"

class MoveCameraAnimation : public Animation
{
public:
	MoveCameraAnimation(Eigen::Affine3f& camera, const Eigen::Vector3f& target) : mCameraStart(camera), mTarget(target), cameraOut(camera) {}

	int getDuration() const override { return 400; }

	void apply(float t) override
	{
		// cubic ease out
		t -= 1;
		cameraOut.translation() = -lerp<Eigen::Vector3f>(-mCameraStart.translation(), mTarget, t*t*t + 1);
	}

private:
	Eigen::Affine3f mCameraStart;
	Eigen::Vector3f mTarget;

	Eigen::Affine3f& cameraOut;
};
