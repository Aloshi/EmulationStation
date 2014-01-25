#pragma once

#include "../GuiComponent.h"
#include "NinePatchComponent.h"

class Font;
class TextCache;

// Used to enter text.
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

	bool isEditing() const;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

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
