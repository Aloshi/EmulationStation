#pragma once

#include "components/ImageComponent.h"

#include "GuiComponent.h"

// A very simple "on/off" switch.
// Should hopefully be switched to use images instead of text in the future.
class SwitchComponent : public GuiComponent
{
public:
	SwitchComponent(Window* window, bool state = false);

	// apis for GuiMetaDataEd
	std::string getValue() const override;
	void setValue(const std::string& value) override;

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;
	void onSizeChanged() override;

	bool getState() const;
	void setState(bool state);

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void onStateChanged();

	ImageComponent mImage;
	bool mState;
};
