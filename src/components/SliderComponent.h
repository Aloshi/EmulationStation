#pragma once

#include "../GuiComponent.h"

class SliderComponent : public GuiComponent
{
public:
	SliderComponent(Window* window, float min, float max);

	void setValue(float val);
	float getValue();

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void onRender() override;
	
	void setSize(Vector2u size);

private:
	float mMin, mMax;
	float mValue;
	float mMoveScale;

	float mMoveRate;
};
