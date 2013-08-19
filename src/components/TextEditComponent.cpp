#include "TextEditComponent.h"
#include "../Log.h"

TextEditComponent::TextEditComponent(Window* window) : GuiComponent(window),
	mBox(window, 0, 0, 0, 0)
{
	addChild(&mBox);
	
	onFocusLost();

	setSize(48, 22);
}

void TextEditComponent::onFocusGained()
{
	mBox.setHorizontalImage(":/glow_hor.png");
	mBox.setVerticalImage(":/glow_vert.png");
	mBox.setBorderColor(0x51CCFFFF);

	SDL_StartTextInput();
}

void TextEditComponent::onFocusLost()
{
	mBox.setHorizontalImage(":/glow_off_hor.png");
	mBox.setVerticalImage(":/glow_off_vert.png");
	mBox.setBorderColor(0xFFFFFFFF);

	SDL_StopTextInput();
}

void TextEditComponent::onSizeChanged()
{
	mBox.setSize(mSize);
}

void TextEditComponent::setValue(const std::string& val)
{
	mText = val;
}

std::string TextEditComponent::getValue() const
{
	return mText;
}

bool TextEditComponent::input(InputConfig* config, Input input)
{
	return GuiComponent::input(config, input);
}
