#include "GuiSettingsMenu.h"
#include "../Renderer.h"
#include "../Settings.h"
#include "../VolumeControl.h"

#include "../scrapers/TheArchiveScraper.h"
#include "../scrapers/GamesDBScraper.h"

GuiSettingsMenu::GuiSettingsMenu(Window* window) : GuiComponent(window), 
	mList(window, Eigen::Vector2i(2, 5)), 
	mBox(mWindow, ":/frame.png", 0x444444FF),
	mDrawFramerateSwitch(window),
	mVolumeSlider(window, 0, 100, 1),
	mDisableSoundsSwitch(window, false),
	mSaveLabel(window),
	mScraperOptList(window)
{
	loadStates();

	addChild(&mBox);
	addChild(&mList);

	mList.setPosition(Renderer::getScreenWidth() / 4.0f, 0);

	using namespace Eigen;

	//drawFramerate label
	TextComponent* label = new TextComponent(mWindow);
	label->setText("Draw Framerate: ");
	label->setColor(0x0000FFFF);
	mList.setEntry(Vector2i(0, 0), Vector2i(1, 1), label, false, ComponentListComponent::AlignRight, Matrix<bool, 1, 2>(true, true));
	mLabels.push_back(label);

	//drawFramerate switch
	mList.setEntry(Vector2i(1, 0), Vector2i(1, 1), &mDrawFramerateSwitch, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//volume label
	label = new TextComponent(mWindow);
	label->setText("System volume: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), label, false, ComponentListComponent::AlignRight, Matrix<bool, 1, 2>(true, true));

	//volume slider
	mList.setEntry(Vector2i(1, 1), Vector2i(1, 1), &mVolumeSlider, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//disable sounds
	label = new TextComponent(mWindow);
	label->setText("Disable sounds: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 2), Vector2i(1, 1), label, false, ComponentListComponent::AlignRight, Matrix<bool, 1, 2>(true, true));

	mList.setEntry(Vector2i(1, 2), Vector2i(1, 1), &mDisableSoundsSwitch, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//scraper label
	label = new TextComponent(mWindow);
	label->setText("Scraper: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 3), Vector2i(1, 1), label, false, ComponentListComponent::AlignRight);

	//fill scraper list
	std::vector< std::shared_ptr<Scraper> > scrapers;
	scrapers.push_back(std::shared_ptr<Scraper>(new GamesDBScraper()));
	scrapers.push_back(std::shared_ptr<Scraper>(new TheArchiveScraper()));
	mScraperOptList.populate(scrapers, [&] (const std::shared_ptr<Scraper>& s) {
		return mScraperOptList.makeEntry(s->getName(), 0x00FF00FF, s, s->getName() == Settings::getInstance()->getScraper()->getName());
	} );

	mList.setEntry(Vector2i(1, 3), Vector2i(1, 1), &mScraperOptList, true, ComponentListComponent::AlignCenter);

	//save label
	mSaveLabel.setText("SAVE");
	mSaveLabel.setColor(0x000000FF);
	mList.setEntry(Vector2i(0, 4), Vector2i(2, 1), &mSaveLabel, true, ComponentListComponent::AlignCenter, Matrix<bool, 1, 2>(false, true));

	//center list
	mList.setPosition(Renderer::getScreenWidth() / 2 - mList.getSize().x() / 2, Renderer::getScreenHeight() / 2 - mList.getSize().y() / 2);

	//set up borders/background
	mBox.fitTo(mList.getSize(), mList.getPosition(), Eigen::Vector2f(8, 8));
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

	mDisableSoundsSwitch.setState(s->getBool("DISABLESOUNDS"));
}

void GuiSettingsMenu::applyStates()
{
	Settings* s = Settings::getInstance();
	s->setBool("DRAWFRAMERATE", mDrawFramerateSwitch.getState());

	VolumeControl::getInstance()->setVolume((int)mVolumeSlider.getValue());

	s->setBool("DISABLESOUNDS", mDisableSoundsSwitch.getState());

	if(mScraperOptList.getSelected().size() > 0)
		s->setScraper(mScraperOptList.getSelected()[0]->object);

	s->saveFile();
}
