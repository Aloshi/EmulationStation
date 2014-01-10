#pragma once

#include "../GuiComponent.h"

class TextCache;
class Font;

class SliderComponent : public GuiComponent
{
public:
	//Minimum value (far left of the slider), maximum value (far right of the slider), increment size (how much just pressing L/R moves by), unit to display (optional).
	SliderComponent(Window* window, float min, float max, float increment, const std::string& suffix = "");

	void setValue(float val);
	float getValue();

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;
	
	void onSizeChanged() override;

private:
	void onValueChanged();

	float mMin, mMax;
	float mValue;
	float mIncrement;
	float mMoveScale;
	int mRepeatWaitTimer;

	std::string mSuffix;
	std::shared_ptr<Font> mFont;
	std::shared_ptr<TextCache> mValueCache;

	float mMoveRate;
};
