#pragma once

#include "../GuiComponent.h"
#include "GuiBox.h"

class TextEditComponent : public GuiComponent
{
public:
	TextEditComponent(Window* window);

	bool input(InputConfig* config, Input input) override;

	void onFocusGained() override;
	void onFocusLost() override;

	void onSizeChanged() override;

	void setValue(const std::string& val) override;
	std::string getValue() const override;
private:
	std::string mText;
	bool mAllowNewlines;

	GuiBox mBox;
};
