#pragma once

#include "../GuiComponent.h"
#include "GuiBox.h"

class Font;

class TextEditComponent : public GuiComponent
{
public:
	TextEditComponent(Window* window);

	void textInput(const char* text) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void onFocusGained() override;
	void onFocusLost() override;

	void onSizeChanged() override;

	void setValue(const std::string& val) override;
	std::string getValue() const override;

private:
	std::string mText;
	bool mFocused;

	std::shared_ptr<Font> getFont();

	GuiBox mBox;
};
