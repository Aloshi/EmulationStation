#include "GuiSettingsMenu.h"
#include "../Renderer.h"
#include "../Settings.h"
#include "../VolumeControl.h"

#include "../scrapers/TheArchiveScraper.h"
#include "../scrapers/GamesDBScraper.h"

GuiSettingsMenu::GuiSettingsMenu(Window* window) : GuiComponent(window), 
	mList(window, Eigen::Vector2i(2, 8)), 
	mBox(mWindow, ":/frame.png", 0x444444FF),
	mDrawFramerateSwitch(window),
	mVolumeSlider(window, 0, 100, 1, "%"),
	mDisableSoundsSwitch(window, false),
	mScraperOptList(window), 
	mScrapeRatingsSwitch(window), 
	mDimSlider(window, 0, 60, 1, "s"),
	mDisableHelpSwitch(window),
	mSaveButton(window)
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
	mList.setEntry(Vector2i(0, 0), Vector2i(1, 1), label, false, ComponentGrid::AlignRight, Matrix<bool, 1, 2>(true, true));
	mLabels.push_back(label);

	//drawFramerate switch
	mList.setEntry(Vector2i(1, 0), Vector2i(1, 1), &mDrawFramerateSwitch, true, ComponentGrid::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//volume label
	label = new TextComponent(mWindow);
	label->setText("System volume: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), label, false, ComponentGrid::AlignRight, Matrix<bool, 1, 2>(true, true));

	//volume slider
	mList.setEntry(Vector2i(1, 1), Vector2i(1, 1), &mVolumeSlider, true, ComponentGrid::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//disable sounds
	label = new TextComponent(mWindow);
	label->setText("Disable sounds: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 2), Vector2i(1, 1), label, false, ComponentGrid::AlignRight, Matrix<bool, 1, 2>(true, true));

	mList.setEntry(Vector2i(1, 2), Vector2i(1, 1), &mDisableSoundsSwitch, true, ComponentGrid::AlignCenter, Matrix<bool, 1, 2>(true, true));

	//scraper label
	label = new TextComponent(mWindow);
	label->setText("Scraper: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 3), Vector2i(1, 1), label, false, ComponentGrid::AlignRight);

	//fill scraper list
	std::vector< std::shared_ptr<Scraper> > scrapers;
	scrapers.push_back(std::shared_ptr<Scraper>(new GamesDBScraper()));
	scrapers.push_back(std::shared_ptr<Scraper>(new TheArchiveScraper()));
	mScraperOptList.populate(scrapers, [&] (const std::shared_ptr<Scraper>& s) {
		return mScraperOptList.makeEntry(s->getName(), s, s->getName() == Settings::getInstance()->getScraper()->getName());
	} );

	mList.setEntry(Vector2i(1, 3), Vector2i(1, 1), &mScraperOptList, true, ComponentGrid::AlignCenter);

	//scrape ratings label
	label = new TextComponent(mWindow);
	label->setText("Scrape ratings? ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 4), Vector2i(1, 1), label, false, ComponentGrid::AlignRight);
	
	mList.setEntry(Vector2i(1, 4), Vector2i(1, 1), &mScrapeRatingsSwitch, true, ComponentGrid::AlignCenter);

	// dim time label
	label = new TextComponent(mWindow);
	label->setText("Dim after: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 5), Vector2i(1, 1), label, false, ComponentGrid::AlignRight);
	mList.setEntry(Vector2i(1, 5), Vector2i(1, 1), &mDimSlider, true, ComponentGrid::AlignCenter);

	// disable help switch
	label = new TextComponent(mWindow);
	label->setText("Disable help: ");
	label->setColor(0x0000FFFF);
	mLabels.push_back(label);
	mList.setEntry(Vector2i(0, 6), Vector2i(1, 1), label, false, ComponentGrid::AlignRight);
	mList.setEntry(Vector2i(1, 6), Vector2i(1, 1), &mDisableHelpSwitch, true, ComponentGrid::AlignCenter);

	//save button
	mSaveButton.setText("SAVE", "apply & save", 0x00FF00FF);
	mSaveButton.setPressedFunc([this] () { applyStates(); delete this; });
	mList.setEntry(Vector2i(0, 7), Vector2i(2, 1), &mSaveButton, true, ComponentGrid::AlignCenter, Matrix<bool, 1, 2>(false, true));

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

	//cancel if b is pressed
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

	mVolumeSlider.setValue((float)VolumeControl::getInstance()->getVolume());

	mDisableSoundsSwitch.setState(s->getBool("DISABLESOUNDS"));

	mScrapeRatingsSwitch.setState(s->getBool("ScrapeRatings"));

	mDimSlider.setValue((float)(s->getInt("DIMTIME") / 1000));

	mDisableHelpSwitch.setState(s->getBool("DISABLEHELP"));
}

void GuiSettingsMenu::applyStates()
{
	Settings* s = Settings::getInstance();
	s->setBool("DRAWFRAMERATE", mDrawFramerateSwitch.getState());

	VolumeControl::getInstance()->setVolume((int)mVolumeSlider.getValue());

	s->setBool("DISABLESOUNDS", mDisableSoundsSwitch.getState());

	if(mScraperOptList.getSelected().size() > 0)
		s->setScraper(mScraperOptList.getSelected()[0]->object);

	s->setBool("ScrapeRatings", mScrapeRatingsSwitch.getState());

	s->setInt("DIMTIME", (int)(mDimSlider.getValue() * 1000));

	s->setBool("DISABLEHELP", mDisableHelpSwitch.getState());

	s->saveFile();
}

std::vector<HelpPrompt> GuiSettingsMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mList.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "discard changes"));
	return prompts;
}
