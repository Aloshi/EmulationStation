#include "GuiSettingsMenu.h"
#include "../Renderer.h"
#include "../Settings.h"
#include "SliderComponent.h"

GuiSettingsMenu::GuiSettingsMenu(Window* window) : GuiComponent(window), 
	mList(window, Vector2u(2, 3)), 
	mDrawFramerateSwitch(window)
{
	addChild(&mList);

	mList.setOffset(Renderer::getScreenWidth() / 4, 0);

	TextComponent* label = new TextComponent(mWindow);
	label->setText("Draw Framerate: ");
	label->setColor(0x0000FFFF);
	mList.setEntry(Vector2u(0, 0), Vector2u(1, 1), label, false, ComponentListComponent::AlignRight, Vector2<bool>(true, true));
	mLabels.push_back(label);

	//drawFramerate switch
	mList.setEntry(Vector2u(1, 0), Vector2u(1, 1), &mDrawFramerateSwitch, true, ComponentListComponent::AlignCenter, Vector2<bool>(true, true));

	label = new TextComponent(mWindow);
	label->setText("Volume: ");
	label->setColor(0x0000FFFF);
	mList.setEntry(Vector2u(0, 1), Vector2u(1, 1), label, false, ComponentListComponent::AlignRight, Vector2<bool>(true, true));

	//volume slider
	SliderComponent* slider = new SliderComponent(mWindow, 0, 1);
	mList.setEntry(Vector2u(1, 1), Vector2u(1, 1), slider, true, ComponentListComponent::AlignCenter, Vector2<bool>(true, true));

	label = new TextComponent(mWindow);
	label->setText("B TO CLOSE");
	label->setColor(0x00FF00FF);
	mList.setEntry(Vector2u(0, 2), Vector2u(2, 1), label, true, ComponentListComponent::AlignCenter, Vector2<bool>(false, true));
	mLabels.push_back(label);

	mList.setOffset(Renderer::getScreenWidth() / 2 - mList.getSize().x / 2, 0);

	loadStates();
}

GuiSettingsMenu::~GuiSettingsMenu()
{
	for(auto iter = mLabels.begin(); iter != mLabels.end(); iter++)
	{
		delete *iter;
	}
}

bool GuiSettingsMenu::input(InputConfig* config, Input input)
{
	//let our children (read: list) go first
	if(GuiComponent::input(config, input))
		return true;

	if(config->isMappedTo("b", input) && input.value)
	{
		delete this;
		return true;
	}

	return false;
}

void GuiSettingsMenu::loadStates()
{
	Settings* s = Settings::getInstance();
	mDrawFramerateSwitch.setState(s->getBool("DRAWFRAMERATE"));
}
