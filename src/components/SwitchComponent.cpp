#include "SwitchComponent.h"
#include "../Renderer.h"
#include "../Font.h"
#include "../Window.h"

SwitchComponent::SwitchComponent(Window* window, bool state) : GuiComponent(window), mState(state)
{
	//mSize = Vector2u((unsigned int)(Renderer::getScreenWidth() * 0.05), 
	//	(unsigned int)(Renderer::getScreenHeight() * 0.05));

	Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM)->sizeText("OFF", (int*)&mSize.x, (int*)&mSize.y);
}

bool SwitchComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value)
	{
		mState = !mState;
		return true;
	}

	return false;
}

void SwitchComponent::onRender()
{
	Renderer::pushClipRect(getGlobalOffset(), getSize());

	Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM)->drawText(mState ? "ON" : "OFF", 0, 0, mState ? 0x00FF00FF : 0xFF0000FF);

	Renderer::popClipRect();

	GuiComponent::onRender();
}

bool SwitchComponent::getState()
{
	return mState;
}

void SwitchComponent::setState(bool state)
{
	mState = state;
}
