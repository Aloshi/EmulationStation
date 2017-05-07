#pragma once

#include "GuiComponent.h"
#include "components/ImageComponent.h"

// A very simple "on/off" switch.
// Should hopefully be switched to use images instead of text in the future.
class SwitchComponent : public GuiComponent
{
public:
	SwitchComponent(Window* window, bool state = false);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;
	void onSizeChanged() override;

	bool getState() const;
	std::string getValue() const;
	void setState(bool state);
	void setValue(const std::string& statestring) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;
	private:
	void onStateChanged();

	ImageComponent mImage;
	bool mState;
};
