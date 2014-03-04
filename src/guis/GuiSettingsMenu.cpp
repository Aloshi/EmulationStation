#include "GuiSettingsMenu.h"
#include "../Renderer.h"
#include "../Settings.h"
#include "../VolumeControl.h"
#include "../Log.h"
#include "../scrapers/TheArchiveScraper.h"
#include "../scrapers/GamesDBScraper.h"

GuiSettingsMenu::GuiSettingsMenu(Window* window) : GuiComponent(window),
	mMenu(mWindow, "SETTINGS")
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	// center menu
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, (mSize.y() - mMenu.getSize().y()) / 2);

	using namespace Eigen;

	Settings* s = Settings::getInstance();

	// framerate
	auto framerate = std::make_shared<SwitchComponent>(mWindow);
	framerate->setState(s->getBool("DRAWFRAMERATE"));
	addSetting("Draw framerate:", framerate, 
		[framerate] { Settings::getInstance()->setBool("DRAWFRAMERATE", framerate->getState()); });

	// volume
	auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
	volume->setValue((float)VolumeControl::getInstance()->getVolume());
	addSetting("System volume:", volume,
		[volume] { VolumeControl::getInstance()->setVolume((int)volume->getValue()); });

	// disable sounds
	auto sound_disable = std::make_shared<SwitchComponent>(mWindow);
	sound_disable->setState(s->getBool("DISABLESOUNDS"));
	addSetting("Disable sound:", sound_disable, 
		[sound_disable] { Settings::getInstance()->setBool("DISABLESOUNDS", sound_disable->getState()); });

	// scraper
	auto scraper_list = std::make_shared< OptionListComponent< std::shared_ptr<Scraper> > >(mWindow, false);
	std::vector< std::shared_ptr<Scraper> > scrapers;
	scrapers.push_back(std::make_shared<GamesDBScraper>());
	scrapers.push_back(std::make_shared<TheArchiveScraper>());
	scraper_list->populate(scrapers, [&] (const std::shared_ptr<Scraper>& sc) {
		return scraper_list->makeEntry(sc->getName(), sc, sc->getName() == Settings::getInstance()->getScraper()->getName());
	});
	addSetting("Scraper:", scraper_list,
		[scraper_list] { 
			if(scraper_list->getSelected().size() > 0) 
				Settings::getInstance()->setScraper(scraper_list->getSelected()[0]->object);
	});

	// scrape ratings
	auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
	scrape_ratings->setState(s->getBool("ScrapeRatings"));
	addSetting("Scrape ratings?", scrape_ratings,
		[scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

	// dim time
	auto dim_time = std::make_shared<SliderComponent>(mWindow, 0.f, 1200.f, 30.f, "s");
	dim_time->setValue((float)(s->getInt("DIMTIME") / 1000));
	addSetting("Dim screen after:", dim_time,
		[dim_time] { Settings::getInstance()->setInt("DIMTIME", (int)(dim_time->getValue() * 1000)); });

	// disable help
	auto disable_help = std::make_shared<SwitchComponent>(mWindow);
	disable_help->setState(s->getBool("DISABLEHELP"));
	addSetting("Disable help:", disable_help,
		[disable_help] { Settings::getInstance()->setBool("DISABLEHELP", disable_help->getState()); });

	// save button
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, "SAVE", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.makeAcceptInputHandler(std::bind(&GuiSettingsMenu::save, this));
	mMenu.addRow(row);

	addChild(&mMenu);
}

void GuiSettingsMenu::addSetting(const char* label, const std::shared_ptr<GuiComponent>& comp, const std::function<void()>& saveFunc)
{
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, label, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.addElement(comp, false);
	mApplyFuncs.push_back(saveFunc);
	mMenu.addRow(row);
}

bool GuiSettingsMenu::input(InputConfig* config, Input input)
{
	//let our children (read: list) go first
	if(GuiComponent::input(config, input))
		return true;

	//cancel if b is pressed
	if(config->isMappedTo("b", input) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

void GuiSettingsMenu::save()
{
	LOG(LogInfo) << "saving";

	for(auto it = mApplyFuncs.begin(); it != mApplyFuncs.end(); it++)
		(*it)();

	Settings::getInstance()->saveFile();

	delete this;
}

std::vector<HelpPrompt> GuiSettingsMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "discard changes"));
	return prompts;
}
