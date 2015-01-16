#include "EmulationStation.h"
#include "guis/GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "RetroboxSystem.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiUpdate.h"
#include "views/ViewController.h"
#include "AudioManager.h"

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "VolumeControl.h"
#include "scrapers/GamesDBScraper.h"
#include "scrapers/TheArchiveScraper.h"

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU"), mVersion(window)
{
	// MAIN MENU

	// SCRAPER >
	// SOUND SETTINGS >
	// UI SETTINGS >
	// CONFIGURE INPUT >
	// QUIT >

	// [version]
//       	addEntry("INFOS", 0x777777FF, true, 
//		[this] { 
//			mWindow->pushGui(new GuiInfo(mWindow));
//	});
    
        addEntry("INFOS", 0x777777FF, true, 
		[this] { 
			auto s = new GuiSettings(mWindow, "INFOS");
                        
                        auto version = std::make_shared<TextComponent>(mWindow, RetroboxSystem::getInstance()->getVersion(), Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                        s->addWithLabel("VERSION", version);
                        bool warning = RetroboxSystem::getInstance()->isFreeSpaceLimit();
                        auto space = std::make_shared<TextComponent>(mWindow, RetroboxSystem::getInstance()->getFreeSpaceInfo(), Font::get(FONT_SIZE_MEDIUM), warning ? 0xFF0000FF : 0x777777FF);
                        s->addWithLabel("STORAGE", space);
                        
                        mWindow->pushGui(s);

	});
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
			
			// disable sounds
			auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
			sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
			s->addWithLabel("ENABLE SOUNDS", sounds_enabled);
			s->addSaveFunc([sounds_enabled] { 
                            Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); 
                            if(!sounds_enabled->getState())
                                AudioManager::getInstance()->stopMusic();
                        });
                        
			auto output_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "OUTPUT DEVICE", false);
			
                        std::string currentDevice = Settings::getInstance()->getString("AudioOutputDevice");
                        output_list->add("HDMI", "HDMI", "HDMI" == currentDevice);
                        output_list->add("JACK", "JACK", "JACK" == currentDevice);
                        output_list->add("AUTO", "AUTO", "AUTO" == currentDevice);

			s->addWithLabel("OUTPUT DEVICE", output_list);
			s->addSaveFunc([output_list] { 
                            if(Settings::getInstance()->getString("AudioOutputDevice") != output_list->getSelected()){
                                Settings::getInstance()->setString("AudioOutputDevice", output_list->getSelected()); 
                                RetroboxSystem::getInstance()->setAudioOutputDevice(output_list->getSelected());
                            }
                        });

			mWindow->pushGui(s);
	});

	addEntry("UI SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "UI SETTINGS");

			// screensaver time
			auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
			screensaver_time->setValue((float)(Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
			s->addWithLabel("SCREENSAVER AFTER", screensaver_time);
			s->addSaveFunc([screensaver_time] { Settings::getInstance()->setInt("ScreenSaverTime", (int)round(screensaver_time->getValue()) * (1000 * 60)); });

			// screensaver behavior
			auto screensaver_behavior = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> screensavers;
			screensavers.push_back("dim");
			screensavers.push_back("black");
			for(auto it = screensavers.begin(); it != screensavers.end(); it++)
				screensaver_behavior->add(*it, *it, Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
			s->addWithLabel("SCREENSAVER BEHAVIOR", screensaver_behavior);
			s->addSaveFunc([screensaver_behavior] { Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected()); });

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

			// quick system select (left/right in game list view)
			auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
			quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
			s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
			s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });

			// transition style
			auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> transitions;
			transitions.push_back("fade");
			transitions.push_back("slide");
			for(auto it = transitions.begin(); it != transitions.end(); it++)
				transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
			s->addWithLabel("TRANSITION STYLE", transition_style);
			s->addSaveFunc([transition_style] { Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected()); });

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

			mWindow->pushGui(s);
	});

            // TODO refactoring
        addEntry("CONFIGURE INPUT", 0x777777FF, true, [this] { this->createConfigInput(); });
                
                
	addEntry("UPDATE", 0x777777FF, true, 
		    [this] {
		         Window* window = mWindow;
		        auto s = new GuiUpdate(window);
		        window->pushGui(s);
	});
        
        addEntry("LANGUAGE", 0x777777FF, true, 
		    [this] {
                        Window* window = mWindow;
			auto s = new GuiSettings(window, "LANGUAGE");
			// language choice 
			auto language_choice = std::make_shared< OptionListComponent<std::string> >(window, "LANGUAGE", false);
                        language_choice->add("Français", "fr_FR", Settings::getInstance()->getString("Lang") == "fr_FR");
                        language_choice->add("English", "en_US", Settings::getInstance()->getString("Lang") == "en_US");
                        language_choice->add("Portugues", "pt_BR", Settings::getInstance()->getString("Lang") == "pt_BR");
                        s->addWithLabel("LANGUAGE", language_choice);
			s->addSaveFunc([language_choice, window] {
                            if(Settings::getInstance()->getString("Lang") == language_choice->getSelected()){
                                return;
                            }
                            Settings::getInstance()->setString("Lang", language_choice->getSelected()); 
                            window->pushGui(
                               new GuiMsgBox(window, "THE SYSTEM WILL NOW REBOOT", "OK", 
                               [] {
                                   if(runRestartCommand() != 0)
                                       LOG(LogWarning) << "Reboot terminated with non-zero result!";
                               })
                            );
                        });
                        mWindow->pushGui(s);

	});
        
	addEntry("QUIT", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "QUIT");
			
			Window* window = mWindow;

			ComponentListRow row;
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES", 
				[] { 
					if(runRestartCommand() != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			row.elements.clear();
			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES", 
				[] { 
					if(runShutdownCommand() != 0)
						LOG(LogWarning) << "Shutdown terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);

			/*if(Settings::getInstance()->getBool("ShowExit"))
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
			}*/
			//ViewController::get()->reloadAll();

			mWindow->pushGui(s);
	});

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0xC6C6C6FF);
	mVersion.setText("EMULATIONSTATION V" + strToUpper(PROGRAM_VERSION_STRING));
	mVersion.setAlignment(ALIGN_CENTER);

	addChild(&mMenu);
	addChild(&mVersion);

	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::createConfigInput(){
    
                GuiSettings *  s = new GuiSettings(mWindow, "CONFIGURE INPUT");

                Window* window = mWindow;

                ComponentListRow row;
                row.makeAcceptInputHandler([window, this, s] {

                    window->pushGui(new GuiDetectDevice(window, false, [this, s] {
                        s->setSave(false);
                        delete s;
                        this->createConfigInput();
                    }));
                });
                        
                
                row.addElement(std::make_shared<TextComponent>(window, "CONFIGURE A CONTROLLER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
                s->addRow(row);

                row.elements.clear();

                // Add the default setting

                // Allow selection of a default input for each player
                std::vector<std::shared_ptr<OptionListComponent<std::string>>> options;
                for (int player = 0; player < 4; player++) {
                    std::stringstream sstm;
                    sstm << "INPUT P" << player +1;
                    std::string confName = sstm.str();

                    auto inputOptionList = std::make_shared<OptionListComponent<std::string> >(mWindow, confName, false);
                    options.push_back(inputOptionList);
                    // Checking if a setting has been saved, else setting to default
                    std::string current_p1 = Settings::getInstance()->getString(confName);
                    bool found = false;
                    for (auto it = 0; it < InputManager::getInstance()->getNumJoysticks(); it++) {
                        InputConfig * config = InputManager::getInstance()->getInputConfigByDevice(it);
                        if(!config->isConfigured()) continue;
                        bool ifound = current_p1.compare(config->getDeviceGUIDString()) == 0;
                        std::string displayName = config->getDeviceName();
                        if(displayName.size() > 25){
                            displayName = displayName.substr(0, 22) + "...";
                        }
                        if(ifound) found = true;
                        inputOptionList->add(displayName, config->getDeviceGUIDString(), ifound);
                    }
                    if (current_p1.compare("") == 0 || !found) {
                        Settings::getInstance()->setString(confName, "DEFAULT");
                    }
                    
                    // ADD default config
                    inputOptionList->add("DEFAULT", "", Settings::getInstance()->getString(confName).compare("DEFAULT") == 0);
                    
                    // Populate controllers list
                    s->addWithLabel(confName, inputOptionList);
                             
                }
                s->addSaveFunc([this, options, window] {
                      for (int player = 0; player < 4; player++) {
                            std::stringstream sstm;
                            sstm << "INPUT P" << player+1;
                            std::string confName = sstm.str();

                            auto input_p1 = options.at(player);
                            std::string name;
                            std::string selectedName = input_p1->getSelectedName();

                            if (selectedName.compare("DEFAULT") == 0) {
                                name = "DEFAULT";
                                Settings::getInstance()->setString(confName, name);                            
                            } else {
                                LOG(LogWarning) << "Found the thing ! : name in list  = "<< selectedName;
                                LOG(LogWarning) << "Found the thing ! : guid  = "<< input_p1->getSelected();

                                std::string selectedGuid = input_p1->getSelected();
                                Settings::getInstance()->setString(confName, selectedGuid);
                            }
                        }
                        Settings::getInstance()->saveFile();
                        // TODO RECONFIGURE DEFAULT SETTINGS ON EMULATIONSTATION ??
                });
                /*row.makeAcceptInputHandler([window, this] {
                    std::string command = Settings::getInstance()->getString("GenerateInputConfigScript") + " DEFAULT";
                    for (int player = 0; player < 2; player++) {
                        std::stringstream sstm;
                        sstm << "INPUT P" << player;
                        std::string confName = sstm.str();
                        Settings::getInstance()->setString(confName, "DEFAULT");
                        Settings::getInstance()->saveFile();
                        
                    }
                    if (system(command.c_str()) == 0) {
                        window->pushGui(new GuiMsgBox(window, "CONFIGURATION RETABLIE", "OK"));
                    }else {
                        window->pushGui(new GuiMsgBox(window, "ERREUR"));                      
                    }
                    
                });*/
                //row.addElement(std::make_shared<TextComponent>(window, "REINITIALISER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
                //s->addRow(row);
                row.elements.clear();
                window->pushGui(s);
                
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

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
