#include "EmulationStation.h"
#include "guis/GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "PowerSaver.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScreensaverOptions.h"
#include "guis/GuiCollectionSystemsOptions.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "views/ViewController.h"

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "VolumeControl.h"

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU"), mVersion(window)
{
	// MAIN MENU

	// SCRAPER >
	// SOUND SETTINGS >
	// UI SETTINGS >
	// CONFIGURE INPUT >
	// QUIT >

	// [version]

	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	addEntry("SCRAPER", 0x777777FF, true,
		[this, openScrapeNow] {
			auto s = new GuiSettings(mWindow, "SCRAPER");

			// scrape from
			auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "SCRAPE FROM", false);
			std::vector<std::string> scrapers = getScraperList();
			for(auto it = scrapers.begin(); it != scrapers.end(); it++)
				scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

			s->addWithLabel("SCRAPE FROM", scraper_list);
			s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

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
			auto bracket = makeArrow(mWindow);
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
			s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)round(volume->getValue())); });

			#ifdef _RPI_
				// volume control device
				auto vol_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "AUDIO DEVICE", false);
				std::vector<std::string> transitions;
				transitions.push_back("PCM");
				transitions.push_back("Speaker");
				transitions.push_back("Master");
				for(auto it = transitions.begin(); it != transitions.end(); it++)
					vol_dev->add(*it, *it, Settings::getInstance()->getString("AudioDevice") == *it);
				s->addWithLabel("AUDIO DEVICE", vol_dev);
				s->addSaveFunc([vol_dev] {
					Settings::getInstance()->setString("AudioDevice", vol_dev->getSelected());
					VolumeControl::getInstance()->deinit();
					VolumeControl::getInstance()->init();
				});
			#endif

			// disable sounds
			auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
			sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
			s->addWithLabel("ENABLE NAVIGATION SOUNDS", sounds_enabled);
			s->addSaveFunc([sounds_enabled] { Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); });

			auto video_audio = std::make_shared<SwitchComponent>(mWindow);
			video_audio->setState(Settings::getInstance()->getBool("VideoAudio"));
			s->addWithLabel("ENABLE VIDEO AUDIO", video_audio);
			s->addSaveFunc([video_audio] { Settings::getInstance()->setBool("VideoAudio", video_audio->getState()); });

#ifdef _RPI_
			// OMX player Audio Device
			auto omx_audio_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "OMX PLAYER AUDIO DEVICE", false);
			std::vector<std::string> devices;
			devices.push_back("local");
			devices.push_back("hdmi");
			devices.push_back("both");
			// USB audio
			devices.push_back("alsa:hw:0,0");
			devices.push_back("alsa:hw:1,0");
			for (auto it = devices.begin(); it != devices.end(); it++)
				omx_audio_dev->add(*it, *it, Settings::getInstance()->getString("OMXAudioDev") == *it);
			s->addWithLabel("OMX PLAYER AUDIO DEVICE", omx_audio_dev);
			s->addSaveFunc([omx_audio_dev] {
				if (Settings::getInstance()->getString("OMXAudioDev") != omx_audio_dev->getSelected())
					Settings::getInstance()->setString("OMXAudioDev", omx_audio_dev->getSelected());
			});
#endif

			mWindow->pushGui(s);
	});

	addEntry("UI SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "UI SETTINGS");

			// screensaver time
			auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
			screensaver_time->setValue((float)(Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
			s->addWithLabel("SCREENSAVER AFTER", screensaver_time);
			s->addSaveFunc([screensaver_time] {
				Settings::getInstance()->setInt("ScreenSaverTime", (int)round(screensaver_time->getValue()) * (1000 * 60));
				PowerSaver::updateTimeouts();
			});

			// screensaver behavior
			auto screensaver_behavior = std::make_shared< OptionListComponent<std::string> >(mWindow, "SCREENSAVER BEHAVIOR", false);
			std::vector<std::string> screensavers;
			screensavers.push_back("dim");
			screensavers.push_back("black");
			screensavers.push_back("random video");
			for(auto it = screensavers.begin(); it != screensavers.end(); it++)
				screensaver_behavior->add(*it, *it, Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
			s->addWithLabel("SCREENSAVER BEHAVIOR", screensaver_behavior);
			s->addSaveFunc([this, screensaver_behavior] {
				if (Settings::getInstance()->getString("ScreenSaverBehavior") != "random video" && screensaver_behavior->getSelected() == "random video") {
					// if before it wasn't risky but now there's a risk of problems, show warning
					mWindow->pushGui(new GuiMsgBox(mWindow,
					"The \"Random Video\" screensaver shows videos from your gamelist.\n\nIf you do not have videos, or if in several consecutive attempts the games it selects don't have videos it will default to black.\n\nMore options in the \"UI Settings\" > \"Video Screensaver\" menu.",
						"OK", [] { return; }));
				}
				Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected());
			});

			ComponentListRow row;

			// show filtered menu
			row.elements.clear();
			row.addElement(std::make_shared<TextComponent>(mWindow, "VIDEO SCREENSAVER SETTINGS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			row.addElement(makeArrow(mWindow), false);
			row.makeAcceptInputHandler(std::bind(&GuiMenu::openScreensaverOptions, this));
			s->addRow(row);

			// quick system select (left/right in game list view)
			auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
			quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
			s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
			s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });

			// carousel transition option
			auto move_carousel = std::make_shared<SwitchComponent>(mWindow);
			move_carousel->setState(Settings::getInstance()->getBool("MoveCarousel"));
			s->addWithLabel("CAROUSEL TRANSITIONS", move_carousel);
			s->addSaveFunc([move_carousel] {
				if (move_carousel->getState()
					&& !Settings::getInstance()->getBool("MoveCarousel")
					&& PowerSaver::getMode() == PowerSaver::INSTANT)
				{
					Settings::getInstance()->setString("PowerSaverMode", "default");
					PowerSaver::init();
				}
				Settings::getInstance()->setBool("MoveCarousel", move_carousel->getState());
			});

			// transition style
			auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> transitions;
			transitions.push_back("fade");
			transitions.push_back("slide");
			transitions.push_back("instant");
			for(auto it = transitions.begin(); it != transitions.end(); it++)
				transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
			s->addWithLabel("TRANSITION STYLE", transition_style);
			s->addSaveFunc([transition_style] {
				if (Settings::getInstance()->getString("TransitionStyle") == "instant"
					&& transition_style->getSelected() != "instant"
					&& PowerSaver::getMode() == PowerSaver::INSTANT)
				{
					Settings::getInstance()->setString("PowerSaverMode", "default");
					PowerSaver::init();
				}
				Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected());
			});

			// theme set
			auto themeSets = ThemeData::getThemeSets();

			if(!themeSets.empty())
			{
				auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
				if(selectedSet == themeSets.end())
					selectedSet = themeSets.begin();

				auto theme_set = std::make_shared< OptionListComponent<std::string> >(mWindow, "THEME SET", false);
				for(auto it = themeSets.begin(); it != themeSets.end(); it++)
					theme_set->add(it->first, it->first, it == selectedSet);
				s->addWithLabel("THEME SET", theme_set);

				Window* window = mWindow;
				s->addSaveFunc([window, theme_set]
				{
					bool needReload = false;
					if(Settings::getInstance()->getString("ThemeSet") != theme_set->getSelected())
						needReload = true;

					Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

					if(needReload)
						ViewController::get()->reloadAll(); // TODO - replace this with some sort of signal-based implementation
				});
			}

			// GameList view style
			auto gamelist_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "GAMELIST VIEW STYLE", false);
			std::vector<std::string> styles;
			styles.push_back("automatic");
			styles.push_back("basic");
			styles.push_back("detailed");
			styles.push_back("video");
			for (auto it = styles.begin(); it != styles.end(); it++)
				gamelist_style->add(*it, *it, Settings::getInstance()->getString("GamelistViewStyle") == *it);
			s->addWithLabel("GAMELIST VIEW STYLE", gamelist_style);
			s->addSaveFunc([gamelist_style] {
				bool needReload = false;
				if (Settings::getInstance()->getString("GamelistViewStyle") != gamelist_style->getSelected())
					needReload = true;
				Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected());
				if (needReload)
					ViewController::get()->reloadAll();
			});

			// show help
			auto show_help = std::make_shared<SwitchComponent>(mWindow);
			show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
			s->addWithLabel("ON-SCREEN HELP", show_help);
			s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

			mWindow->pushGui(s);
	});

	addEntry("GAME COLLECTION SETTINGS", 0x777777FF, true,
		[this] { openCollectionSystemSettings();
		});

	addEntry("OTHER SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "OTHER SETTINGS");

			// maximum vram
			auto max_vram = std::make_shared<SliderComponent>(mWindow, 0.f, 1000.f, 10.f, "Mb");
			max_vram->setValue((float)(Settings::getInstance()->getInt("MaxVRAM")));
			s->addWithLabel("VRAM LIMIT", max_vram);
			s->addSaveFunc([max_vram] { Settings::getInstance()->setInt("MaxVRAM", (int)round(max_vram->getValue())); });

			// power saver
			auto power_saver = std::make_shared< OptionListComponent<std::string> >(mWindow, "POWER SAVER MODES", false);
			std::vector<std::string> modes;
			modes.push_back("disabled");
			modes.push_back("default");
			modes.push_back("enhanced");
			modes.push_back("instant");
			for (auto it = modes.begin(); it != modes.end(); it++)
				power_saver->add(*it, *it, Settings::getInstance()->getString("PowerSaverMode") == *it);
			s->addWithLabel("POWER SAVER MODES", power_saver);
			s->addSaveFunc([this, power_saver] {
				if (Settings::getInstance()->getString("PowerSaverMode") != "instant" && power_saver->getSelected() == "instant") {
					Settings::getInstance()->setString("TransitionStyle", "instant");
					Settings::getInstance()->setBool("MoveCarousel", false);
				}
				Settings::getInstance()->setString("PowerSaverMode", power_saver->getSelected());
				PowerSaver::init();
			});

			// gamelists
			auto save_gamelists = std::make_shared<SwitchComponent>(mWindow);
			save_gamelists->setState(Settings::getInstance()->getBool("SaveGamelistsOnExit"));
			s->addWithLabel("SAVE METADATA ON EXIT", save_gamelists);
			s->addSaveFunc([save_gamelists] { Settings::getInstance()->setBool("SaveGamelistsOnExit", save_gamelists->getState()); });

			auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
			parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
			s->addWithLabel("PARSE GAMESLISTS ONLY", parse_gamelists);
			s->addSaveFunc([parse_gamelists] { Settings::getInstance()->setBool("ParseGamelistOnly", parse_gamelists->getState()); });

#ifndef WIN32
			// hidden files
			auto hidden_files = std::make_shared<SwitchComponent>(mWindow);
			hidden_files->setState(Settings::getInstance()->getBool("ShowHiddenFiles"));
			s->addWithLabel("SHOW HIDDEN FILES", hidden_files);
			s->addSaveFunc([hidden_files] { Settings::getInstance()->setBool("ShowHiddenFiles", hidden_files->getState()); });
#endif

#ifdef _RPI_
			// Video Player - VideoOmxPlayer
			auto omx_player = std::make_shared<SwitchComponent>(mWindow);
			omx_player->setState(Settings::getInstance()->getBool("VideoOmxPlayer"));
			s->addWithLabel("USE OMX PLAYER (HW ACCELERATED)", omx_player);
			s->addSaveFunc([omx_player]
			{
				// need to reload all views to re-create the right video components
				bool needReload = false;
				if(Settings::getInstance()->getBool("VideoOmxPlayer") != omx_player->getState())
					needReload = true;

				Settings::getInstance()->setBool("VideoOmxPlayer", omx_player->getState());

				if(needReload)
					ViewController::get()->reloadAll();
			});

#endif

			// framerate
			auto framerate = std::make_shared<SwitchComponent>(mWindow);
			framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
			s->addWithLabel("SHOW FRAMERATE", framerate);
			s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });


			mWindow->pushGui(s);
	});

	addEntry("CONFIGURE INPUT", 0x777777FF, true,
		[this] {
			Window* window = mWindow;
			window->pushGui(new GuiMsgBox(window, "ARE YOU SURE YOU WANT TO CONFIGURE INPUT?", "YES",
				[window] {
					window->pushGui(new GuiDetectDevice(window, false, nullptr));
				}, "NO", nullptr)
			);
	});

	addEntry("QUIT", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "QUIT");

			Window* window = mWindow;

			ComponentListRow row;
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES",
				[] {
					if(quitES("/tmp/es-restart") != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES",
				[] {
					if(quitES("/tmp/es-sysrestart") != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES",
				[] {
					if(quitES("/tmp/es-shutdown") != 0)
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

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0x5E5E5EFF);
	mVersion.setText("EMULATIONSTATION V" + strToUpper(PROGRAM_VERSION_STRING));
	mVersion.setAlignment(ALIGN_CENTER);

	addChild(&mMenu);
	addChild(&mVersion);

	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::openScreensaverOptions() {
	mWindow->pushGui(new GuiScreensaverOptions(mWindow, "VIDEO SCREENSAVER"));
}

void GuiMenu::openCollectionSystemSettings() {
	mWindow->pushGui(new GuiCollectionSystemsOptions(mWindow));
}

void GuiMenu::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);

	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}

	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

HelpStyle GuiMenu::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
