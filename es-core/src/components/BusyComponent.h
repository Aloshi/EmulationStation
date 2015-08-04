#pragma once
#include "GuiComponent.h"
#include "components/ComponentGrid.h"
#include "components/NinePatchComponent.h"

class AnimatedImageComponent;
class TextComponent;

class BusyComponent : public GuiComponent
{
public:
	BusyComponent(Window* window);

	void onSizeChanged() override;

	void reset(); // reset to frame 0

private:
	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<AnimatedImageComponent> mAnimation;
	std::shared_ptr<TextComponent> mText;
};
