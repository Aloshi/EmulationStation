#pragma once

#include "../GuiComponent.h"
#include "NinePatchComponent.h"

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

	bool isMultiline();

	std::string mText;
	bool mFocused;

	bool mEditing;
	Eigen::Vector2f mScrollOffset;
	int mCursor;

	std::shared_ptr<Font> getFont();

	NinePatchComponent mBox;

	std::unique_ptr<TextCache> mTextCache;
};
