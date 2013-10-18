#pragma once

#include "../GuiComponent.h"
#include "ImageComponent.h"

class VerticalImageAutoScrollbox : public GuiComponent
{
public:
	VerticalImageAutoScrollbox(Window* window);

        // reset shown position to top image, reset animation timer
        void reset();
        // 'delay' is the time between the movement animations
        // 'animTime' is how long the animation lasts
	void setAutoScroll(int delay, int animTime);
        // distance between two images in px
        void setBorderSpace(float dist);

        void setAllowImageUpscale(bool allowUpscaling);

        // positions image behind current last image and does an addChild
        void addImage(ImageComponent *img);

	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
        float getAnimTargetPos(unsigned int childNo) const;

	Eigen::Vector2f getContentSize() const;

	Eigen::Vector2d mScrollPos;
	int mAutoScrollDelay;
        int mAnimDuration;
	int mAutoScrollTimer; // how long are we waiting/animating?
        float mBorderSpace;
        bool mAllowUpscaling;
        bool mCentered;
        int mAnimTargetChildNo;;
};
