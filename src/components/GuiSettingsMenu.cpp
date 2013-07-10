#include "GuiSettingsMenu.h"
#include "../Renderer.h"
#include "../Settings.h"
#include "../VolumeControl.h"

GuiSettingsMenu::GuiSettingsMenu(Window* window) : GuiComponent(window), 
	mList(window, Eigen::Vector2i(2, 3)), 
	mDrawFramerateSwitch(window),
	mVolumeSlider(window, 0, 100, 1),
	mSaveLabel(window)
{
	loadStates();

	addChild(&mList);

	mList.setPosition(Renderer::getScreenWidth() / 4.0f, 0);

	using namespace Eigen;

	TextComponent* label = new TextComponent(mWindow);
	label->setText("Draw Framerate: ");
	label->setColor(0x0000FFFF);
	mList.setEntry(Vector2i(0, 0), Vector2i(1, 1), label, false, ComponentListComponent::AlignRight, Matrix<bool, 1, 2>(true, true));
	mLabels.push_back(label);

	//drawFramerate switch
	mList.setEntry(Vector2i(1, 0), Vector2i(1, 1), &mDrawFramerateSwitch, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(true, true));

	label = new TextComponent(mWindow);
	label->setText("System volume: ");
	label->setColor(0x0000FFFF);
	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), label, false, ComponentListComponent::AlignRight, Matrix<bool, 1, 2>(true, true));

	//volume slider
	mList.setEntry(Vector2i(1, 1), Vector2i(1, 1), &mVolumeSlider, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//save label
	mSaveLabel.setText("SAVE");
	mSaveLabel.setColor(0x000000FF);
	mList.setEntry(Vector2i(0, 2), Vector2i(2, 1), &mSaveLabel, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(false, true));

	mList.setPosition(Renderer::getScreenWidth() / 2 - mList.getSize().x() / 2, 0);
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

	if(config->isMappedTo("a", input) && mList.getSelectedComponent() == &mSaveLabel && input.value)
	{
		applyStates();
		delete this;
		return true;
	}

	return false;
}

void GuiSettingsMenu::loadStates()
{
	Settings* s = Settings::getInstance();
	mDrawFramerateSwitch.setState(s->getBool("DRAWFRAMERATE"));

	mVolumeSlider.setValue((float)VolumeControl::getInstance()->getVolume());
}

void GuiSettingsMenu::applyStates()
{
	Settings* s = Settings::getInstance();
	s->setBool("DRAWFRAMERATE", mDrawFramerateSwitch.getState());

	VolumeControl::getInstance()->setVolume((int)mVolumeSlider.getValue());

	s->saveFile();
}
