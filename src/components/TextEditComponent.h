#pragma once

#include "../GuiComponent.h"
#include "GuiBox.h"

class Font;
class TextCache;

class TextEditComponent : public GuiComponent
{
public:
	TextEditComponent(Window* window);
	
	void textInput(const char* text) override;
	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void onFocusGained() override;
	void onFocusLost() override;

	void onSizeChanged() override;

	void setValue(const std::string& val) override;
	std::string getValue() const override;

private:
	void onTextChanged();
	void onCursorChanged();

	std::string mText;
	bool mFocused;

	bool mEditing;
	float mScrollOffset;
	int mCursor;

	std::shared_ptr<Font> getFont();

	GuiBox mBox;

	std::unique_ptr<TextCache> mTextCache;
};
