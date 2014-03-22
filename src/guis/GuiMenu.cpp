#include "GuiMenu.h"
#include "../Window.h"
#include "../Sound.h"
#include "../Log.h"
#include "../Settings.h"
#include "GuiMsgBox.h"
#include "GuiSettings.h"
#include "GuiScraperStart.h"
#include "GuiDetectDevice.h"

#include "../components/ButtonComponent.h"
#include "../components/SwitchComponent.h"
#include "../components/SliderComponent.h"
#include "../components/TextComponent.h"
#include "../components/OptionListComponent.h"
#include "../VolumeControl.h"
#include "../scrapers/GamesDBScraper.h"
#include "../scrapers/TheArchiveScraper.h"

std::shared_ptr<ImageComponent> makeBracket(Window* window)
{
	auto bracket = std::make_shared<ImageComponent>(window);
	bracket->setImage(":/sq_bracket.png");

	// resize
	const float fontHeight = (float)Font::get(FONT_SIZE_MEDIUM)->getHeight();
	if(bracket->getTextureSize().y() > fontHeight)
		bracket->setResize(0, fontHeight);

	return bracket;
}

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU")
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	// SCRAPER >
	// SOUND SETTINGS >
	// UI SETTINGS >
	// QUIT >
	
	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	addEntry("SCRAPER", 0x777777FF, true, 
		[this, openScrapeNow] { 
			auto s = new GuiSettings(mWindow, "SCRAPER");

			// scrape from
			auto scraper_list = std::make_shared< OptionListComponent< std::shared_ptr<Scraper> > >(mWindow, "SCRAPE FROM", false);
			std::vector< std::shared_ptr<Scraper> > scrapers;
			scrapers.push_back(std::make_shared<GamesDBScraper>());
			scrapers.push_back(std::make_shared<TheArchiveScraper>());

			for(auto it = scrapers.begin(); it != scrapers.end(); it++)
				scraper_list->add((*it)->getName(), *it, (*it)->getName() == Settings::getInstance()->getScraper()->getName());

			s->addWithLabel("SCRAPE FROM", scraper_list);
			s->addSaveFunc([scraper_list] { Settings::getInstance()->setScraper(scraper_list->getSelected()); });

			// scrape ratings
			auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
			scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
			s->addWithLabel("SCRAPE RATINGS", scrape_ratings);
			s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

			// scrape now
			ComponentListRow row;
			std::function<void()> openAndSave = openScrapeNow;
			openAndSave = [s, openAndSave] { s->save(); openAndSave(); };
			row.makeAcceptInputHandler(openAndSave);

			auto scrape_now = std::make_shared<TextComponent>(mWindow, "SCRAPE NOW", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
			auto bracket = makeBracket(mWindow);
			row.addElement(scrape_now, true);
			row.addElement(bracket, false);
			s->addRow(row);

			mWindow->pushGui(s);
	});

	addEntry("SOUND SETTINGS", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "SOUND SETTINGS");

			// volume
			auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
			volume->setValue((float)VolumeControl::getInstance()->getVolume());
			s->addWithLabel("SYSTEM VOLUME", volume);
			s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)volume->getValue()); });
			
			// disable sounds
			auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
			sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
			s->addWithLabel("ENABLE SOUNDS", sounds_enabled);
			s->addSaveFunc([sounds_enabled] { Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); });

			mWindow->pushGui(s);
	});

	addEntry("UI SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "UI SETTINGS");

			// dim time
			auto dim_time = std::make_shared<SliderComponent>(mWindow, 0.f, 1200.f, 30.f, "s");
			dim_time->setValue((float)(Settings::getInstance()->getInt("DimTime") / 1000));
			s->addWithLabel("DIM SCREEN AFTER", dim_time);
			s->addSaveFunc([dim_time] { Settings::getInstance()->setInt("DimTime", (int)(dim_time->getValue() * 1000)); });

			// framerate
			auto framerate = std::make_shared<SwitchComponent>(mWindow);
			framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
			s->addWithLabel("SHOW FRAMERATE", framerate);
			s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });

			// show help
			auto show_help = std::make_shared<SwitchComponent>(mWindow);
			show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
			s->addWithLabel("ON-SCREEN HELP", show_help);
			s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

			// transition style
			auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> transitions;
			transitions.push_back("fade");
			transitions.push_back("slide");
			for(auto it = transitions.begin(); it != transitions.end(); it++)
				transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
			s->addWithLabel("TRANSITION STYLE", transition_style);
			s->addSaveFunc([transition_style] { Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected()); });

			mWindow->pushGui(s);
	});

	addEntry("CONFIGURE INPUT", 0x777777FF, true, 
		[this] { 
			mWindow->pushGui(new GuiDetectDevice(mWindow, false));
	});

	addEntry("QUIT", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "QUIT");
			
			Window* window = mWindow;

			ComponentListRow row;
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES", 
				[] { 
					if(system("sudo shutdown -r now") != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES", 
				[] { 
					if(system("sudo shutdown -h now") != 0)
						LOG(LogWarning) << "Shutdown terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			if(Settings::getInstance()->getBool("ShowExit"))
			{
				row.elements.clear();
				row.makeAcceptInputHandler([window] {
					window->pushGui(new GuiMsgBox(window, "REALLY QUIT?", "YES", 
					[] { 
						SDL_Event ev;
						ev.type = SDL_QUIT;
						SDL_PushEvent(&ev);
					}, "NO", nullptr));
				});
				row.addElement(std::make_shared<TextComponent>(window, "QUIT EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
				s->addRow(row);
			}

			mWindow->pushGui(s);
	});

	addChild(&mMenu);
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeBracket(mWindow);
		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return GuiComponent::input(config, input);
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "move cursor"));
	prompts.push_back(HelpPrompt("a", "accept"));
	return prompts;
}
