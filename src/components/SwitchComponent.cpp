#include "SwitchComponent.h"
#include "../Renderer.h"
#include "../resources/Font.h"
#include "../Window.h"

SwitchComponent::SwitchComponent(Window* window, bool state) : GuiComponent(window), mImage(window), mState(state)
{
	mImage.setImage(":/checkbox_unchecked.svg");

	float height = (float)FONT_SIZE_MEDIUM;
	mImage.setResize(0, height);
	mSize = mImage.getSize();
}

bool SwitchComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value)
	{
		mState = !mState;
		onStateChanged();
		return true;
	}

	return false;
}

void SwitchComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	mImage.render(trans);

	renderChildren(trans);
}

bool SwitchComponent::getState() const
{
	return mState;
}

void SwitchComponent::setState(bool state)
{
	mState = state;
	onStateChanged();
}

void SwitchComponent::onStateChanged()
{
	mImage.setImage(mState ? ":/checkbox_checked.svg" : ":/checkbox_unchecked.svg");
}

std::vector<HelpPrompt> SwitchComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("a", "toggle"));
	return prompts;
}
