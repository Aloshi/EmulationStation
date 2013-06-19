#pragma once

#include "../GuiComponent.h"

class SliderComponent : public GuiComponent
{
public:
	//Minimum value (far left of the slider), maximum value (far right of the slider), increment size (how much just pressing L/R moves by).
	SliderComponent(Window* window, float min, float max, float increment);

	void setValue(float val);
	float getValue();

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void onRender() override;
	
	void setSize(Vector2u size);

private:
	float mMin, mMax;
	float mValue;
	float mIncrement;
	float mMoveScale;
	int mRepeatWaitTimer;

	float mMoveRate;
};
