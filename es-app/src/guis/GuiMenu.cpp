#include <list>
#include <algorithm>
#include <string>
#include <sstream>

#include <functional>

#include "EmulationStation.h"
#include "guis/GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "RecalboxSystem.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiUpdate.h"
#include "guis/GuiRomsManager.h"
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

#include "guis/GuiTextEditPopup.h"
#include "GuiLoading.h"

#include "RecalboxConf.h"

void GuiMenu::createInputTextRow(GuiSettings *gui, const char *title, const char *settingsID, bool password) {
    // LABEL
    Window *window = mWindow;
    ComponentListRow row;

    auto lbl = std::make_shared<TextComponent>(window, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
    row.addElement(lbl, true); // label

    std::shared_ptr<GuiComponent> ed;

    ed = std::make_shared<TextComponent>(window, ((password &&
            RecalboxConf::getInstance()->get(settingsID) != "")
                                                  ? "*********" : RecalboxConf::getInstance()->get(
                    settingsID)), Font::get(FONT_SIZE_MEDIUM, FONT_PATH_LIGHT), 0x777777FF, ALIGN_RIGHT);
    row.addElement(ed, true);

    auto spacer = std::make_shared<GuiComponent>(mWindow);
    spacer->setSize(Renderer::getScreenWidth() * 0.005f, 0);
    row.addElement(spacer, false);

    auto bracket = std::make_shared<ImageComponent>(mWindow);
    bracket->setImage(":/arrow.svg");
    bracket->setResize(Eigen::Vector2f(0, lbl->getFont()->getLetterHeight()));
    row.addElement(bracket, false);

    auto updateVal = [ed, settingsID, password](const std::string &newVal) {
        if (!password)
            ed->setValue(newVal);
        else {
            ed->setValue("*********");
        }
        RecalboxConf::getInstance()->set(settingsID, newVal);
    }; // ok callback (apply new value to ed)
    row.makeAcceptInputHandler([this, title, updateVal, settingsID] {
        mWindow->pushGui(
                new GuiTextEditPopup(mWindow, title, RecalboxConf::getInstance()->get(settingsID),
                                     updateVal, false));
    });
    gui->addRow(row);
}

GuiMenu::GuiMenu(Window *window) : GuiComponent(window), mMenu(window, "MAIN MENU"), mVersion(window) {
    // MAIN MENU

    // SCRAPER >
    // SOUND SETTINGS >
    // UI SETTINGS >
    // CONFIGURE INPUT >
    // QUIT >
    if (RecalboxConf::getInstance()->get("kodi.enabled") == "1") {
        addEntry("KODI MEDIA CENTER", 0x777777FF, true,
                 [this] {
                     Window *window = mWindow;

                     if (!RecalboxSystem::getInstance()->launchKodi(window)) {
                         LOG(LogWarning) << "Shutdown terminated with non-zero result!";
                     }
                 });
    }
    if (Settings::getInstance()->getBool("RomsManager")) {
		addEntry("ROMS MANAGER", 0x777777FF, true, [this] {
			mWindow->pushGui(new GuiRomsManager(mWindow));
		});
	}
    if(RecalboxConf::getInstance()->get("system.es.menu") != "bartop"){
    addEntry("SYSTEM SETTINGS", 0x777777FF, true,
             [this] {
                 Window *window = mWindow;

                 auto s = new GuiSettings(mWindow, "SYSTEM SETTINGS");

                 auto version = std::make_shared<TextComponent>(mWindow, RecalboxSystem::getInstance()->getVersion(),
                                                                Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                 s->addWithLabel("VERSION", version);
                 bool warning = RecalboxSystem::getInstance()->isFreeSpaceLimit();
                 auto space = std::make_shared<TextComponent>(mWindow,
                                                              RecalboxSystem::getInstance()->getFreeSpaceInfo(),
                                                              Font::get(FONT_SIZE_MEDIUM),
                                                              warning ? 0xFF0000FF : 0x777777FF);
                 s->addWithLabel("STORAGE", space);

                 // language choice
                 auto language_choice = std::make_shared<OptionListComponent<std::string> >(window, "LANGUAGE", false);
                 std::string language = RecalboxConf::getInstance()->get("system.language");
                 if (language.empty()) language = "en_US";
                 language_choice->add("Français", "fr_FR", language == "fr_FR");
                 language_choice->add("English", "en_US", language == "en_US");
                 language_choice->add("Portugues", "pt_BR", language == "pt_BR");
                 language_choice->add("Español", "es_ES", language == "es_ES");
                 language_choice->add("Deutsch", "de_DE", language == "de_DE");
                 language_choice->add("Italiano", "it_IT", language == "it_IT");
                 language_choice->add("Basque", "eu_ES", language == "eu_ES");
                 language_choice->add("Türkçe", "tr_TR", language == "tr_TR");
                 language_choice->add("Chinese", "zh_CN", language == "zh_CN");

                 s->addWithLabel("LANGUAGE", language_choice);

                 // Overclock choice
                 auto overclock_choice = std::make_shared<OptionListComponent<std::string> >(window, "OVERCLOCK",
                                                                                             false);
#ifdef RPI_VERSION
    #if RPI_VERSION == 1
                 std::string currentOverclock = Settings::getInstance()->getString("Overclock");
                 overclock_choice->add("EXTREM (1100Mhz)", "extrem", currentOverclock == "extrem");
                 overclock_choice->add("TURBO (1000Mhz)", "turbo", currentOverclock == "turbo");
                 overclock_choice->add("HIGH (950Mhz)", "high", currentOverclock == "high");
                 overclock_choice->add("NONE (700Mhz)", "none", currentOverclock == "none");
    #else
                 std::string currentOverclock = Settings::getInstance()->getString("Overclock-rpi2");
                 overclock_choice->add("RPI2 (1050Mhz)", "rpi2", currentOverclock == "rpi2");
                 overclock_choice->add("NONE (900Mhz)", "none-rpi2", currentOverclock == "none-rpi2");
    #endif
#else
                 overclock_choice->add("None", "none", true);
#endif
                 s->addWithLabel("OVERCLOCK", overclock_choice);

                 // Updates
                 {
                     ComponentListRow row;
                     std::function<void()> openGuiD = [this] {
                         GuiSettings *updateGui = new GuiSettings(mWindow, "UPDATES");
                         // Enable updates
                         auto updates_enabled = std::make_shared<SwitchComponent>(mWindow);
                         updates_enabled->setState(
                                 RecalboxConf::getInstance()->get("updates.enabled") == "1");
                         updateGui->addWithLabel("AUTO UPDATES", updates_enabled);

                         // Start update
                         {
                             ComponentListRow updateRow;
                             std::function<void()> openGui = [this] { mWindow->pushGui(new GuiUpdate(mWindow)); };
                             updateRow.makeAcceptInputHandler(openGui);
                             auto update = std::make_shared<TextComponent>(mWindow, "START UPDATE",
                                                                           Font::get(FONT_SIZE_MEDIUM),
                                                                           0x777777FF);
                             auto bracket = makeArrow(mWindow);
                             updateRow.addElement(update, true);
                             updateRow.addElement(bracket, false);
                             updateGui->addRow(updateRow);
                         }
                         updateGui->addSaveFunc([updates_enabled] {
                             RecalboxConf::getInstance()->set("updates.enabled",
                                                                              updates_enabled->getState() ? "1" : "0");
                             RecalboxConf::getInstance()->saveRecalboxConf();
                         });
                         mWindow->pushGui(updateGui);

                     };
                     row.makeAcceptInputHandler(openGuiD);
                     auto update = std::make_shared<TextComponent>(mWindow, "UPDATES", Font::get(FONT_SIZE_MEDIUM),
                                                                   0x777777FF);
                     auto bracket = makeArrow(mWindow);
                     row.addElement(update, true);
                     row.addElement(bracket, false);
                     s->addRow(row);
                 }


                 //Kodi
                 {
                     ComponentListRow row;
                     std::function<void()> openGui = [this] {
                         GuiSettings *kodiGui = new GuiSettings(mWindow, "KODI SETTINGS");
                         auto kodiEnabled = std::make_shared<SwitchComponent>(mWindow);
                         kodiEnabled->setState(RecalboxConf::getInstance()->get("kodi.enabled") == "1");
                         kodiGui->addWithLabel("ENABLE KODI", kodiEnabled);
                         auto kodiAtStart = std::make_shared<SwitchComponent>(mWindow);
                         kodiAtStart->setState(
                                 RecalboxConf::getInstance()->get("kodi.atstartup") == "1");
                         kodiGui->addWithLabel("KODI AT START", kodiAtStart);
                         auto kodiX = std::make_shared<SwitchComponent>(mWindow);
                         kodiX->setState(RecalboxConf::getInstance()->get("kodi.xbutton") == "1");
                         kodiGui->addWithLabel("START KODI WITH X", kodiX);
                         kodiGui->addSaveFunc([kodiEnabled, kodiAtStart, kodiX] {
                             RecalboxConf::getInstance()->set("kodi.enabled",
                                                                              kodiEnabled->getState() ? "1" : "0");
                             RecalboxConf::getInstance()->set("kodi.atstartup",
                                                                              kodiAtStart->getState() ? "1" : "0");
                             RecalboxConf::getInstance()->set("kodi.xbutton",
                                                                              kodiX->getState() ? "1" : "0");
                             RecalboxConf::getInstance()->saveRecalboxConf();
                         });
                         mWindow->pushGui(kodiGui);
                     };
                     row.makeAcceptInputHandler(openGui);
                     auto kodiSettings = std::make_shared<TextComponent>(mWindow, "KODI SETTINGS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                     auto bracket = makeArrow(mWindow);
                     row.addElement(kodiSettings, true);
                     row.addElement(bracket, false);
                     s->addRow(row);
                 }


                 s->addSaveFunc([overclock_choice, window, language_choice, language] {
                     bool reboot = false;

                     if (Settings::getInstance()->getString("Overclock") != overclock_choice->getSelected()) {
                         Settings::getInstance()->setString("Overclock", overclock_choice->getSelected());
                         RecalboxSystem::getInstance()->setOverclock(overclock_choice->getSelected());
                         reboot = true;
                     }
                     if (language != language_choice->getSelected()) {
                         RecalboxConf::getInstance()->set("system.language",
                                                                          language_choice->getSelected());
                         RecalboxConf::getInstance()->saveRecalboxConf();
                         reboot = true;
                     }
                     if (reboot) {
                         window->pushGui(
                                 new GuiMsgBox(window, "THE SYSTEM WILL NOW REBOOT", "OK",
                                               [] {
                                                   if (runRestartCommand() != 0) {
                                                       LOG(LogWarning) << "Reboot terminated with non-zero result!";
                                                   }
                                               })
                         );
                     }

                 });
                 mWindow->pushGui(s);

             });
    }

    addEntry("GAMES SETTINGS", 0x777777FF, true,
             [this] {
                 auto s = new GuiSettings(mWindow, "GAMES SETTINGS");
                 // Screen ratio choice
                 auto ratio_choice = std::make_shared<OptionListComponent<std::string> >(mWindow, "GAME RATIO", false);
                 std::string currentRatio = RecalboxConf::getInstance()->get("global.ratio");
                 if (currentRatio.empty()) {
                     currentRatio = std::string("auto");
                 }

                 ratio_choice->add("CUSTOM", "custom", currentRatio == "custom");
                 ratio_choice->add("AUTO", "auto", currentRatio == "auto");
                 ratio_choice->add("4/3", "4/3", currentRatio == "4/3");
                 ratio_choice->add("16/9", "16/9", currentRatio == "16/9");

                 s->addWithLabel("GAME RATIO", ratio_choice);

                 // smoothing
                 auto smoothing_enabled = std::make_shared<SwitchComponent>(mWindow);
                 smoothing_enabled->setState(RecalboxConf::getInstance()->get("global.smooth") == "1");
                 s->addWithLabel("SMOOTH GAMES", smoothing_enabled);


                 // rewind
                 auto rewind_enabled = std::make_shared<SwitchComponent>(mWindow);
                 rewind_enabled->setState(RecalboxConf::getInstance()->get("global.rewind") == "1");
                 s->addWithLabel("REWIND", rewind_enabled);

                 // Shaders preset

                 auto shaders_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, "SHADERS SET", false);
                 std::string currentShader = RecalboxConf::getInstance()->get("global.shaderset");
                 if (currentShader.empty()) {
                     currentShader = std::string("nano");
                 }

                 shaders_choices->add("NONE", "none", currentShader == "none");
                 shaders_choices->add("SCANLINES", "scanlines", currentShader == "scanlines");
                 shaders_choices->add("RETRO", "retro", currentShader == "retro");
                 s->addWithLabel("SHADERS SET", shaders_choices);


                 s->addSaveFunc([smoothing_enabled, ratio_choice, rewind_enabled, shaders_choices] {
                     RecalboxConf::getInstance()->set("global.smooth", smoothing_enabled->getState() ? "1" : "0");
                     RecalboxConf::getInstance()->set("global.ratio", ratio_choice->getSelected());
                     RecalboxConf::getInstance()->set("global.rewind", rewind_enabled->getState() ? "1" : "0");
                     RecalboxConf::getInstance()->set("global.shaderset", shaders_choices->getSelected());
                     RecalboxConf::getInstance()->saveRecalboxConf();
                 });
                 // reread game list
                 ComponentListRow row;
                 Window *window = mWindow;
		 
                 row.makeAcceptInputHandler([window] {
                     window->pushGui(new GuiMsgBox(window, "REALLY UPDATE GAMES LISTS ?", "YES",
                                                   [] {
						     // todo : add something nice to display here, in case it takes time ?
						     for(auto i = SystemData::sSystemVector.begin(); i != SystemData::sSystemVector.end(); i++)
						       {
							 (*i)->refreshRootFolder();
						       }
						     // not sure that all must be reloaded (games lists, lists counters, what else ?)
						     ViewController::get()->reloadGamesLists();
						   }, "NO", nullptr));
                 });
                 row.addElement(std::make_shared<TextComponent>(window, "UPDATE GAMES LISTS", Font::get(FONT_SIZE_MEDIUM),
                                                                0x777777FF), true);
                 //s->addRow(row);
		 
		 
                 s->addSaveFunc([smoothing_enabled, ratio_choice, rewind_enabled] {
                     RecalboxSystem::getInstance()->setRecalboxConfig("global.smooth", smoothing_enabled->getState() ? "1" : "0");
                     RecalboxSystem::getInstance()->setRecalboxConfig("global.ratio", ratio_choice->getSelected());
                     RecalboxSystem::getInstance()->setRecalboxConfig("global.rewind", rewind_enabled->getState() ? "1" : "0");

                 });
                 mWindow->pushGui(s);
             });

    if(RecalboxConf::getInstance()->get("system.es.menu") != "bartop"){
        addEntry("CONTROLLERS SETTINGS", 0x777777FF, true, [this] { this->createConfigInput(); });
    }
    if(RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {

        addEntry("UI SETTINGS", 0x777777FF, true,
                 [this] {
                     auto s = new GuiSettings(mWindow, "UI SETTINGS");

                     // overscan
                     auto overscan_enabled = std::make_shared<SwitchComponent>(mWindow);
                     overscan_enabled->setState(Settings::getInstance()->getBool("Overscan"));
                     s->addWithLabel("OVERSCAN", overscan_enabled);
                     s->addSaveFunc([overscan_enabled] {
                         if (Settings::getInstance()->getBool("Overscan") != overscan_enabled->getState()) {
                             Settings::getInstance()->setBool("Overscan", overscan_enabled->getState());
                             RecalboxSystem::getInstance()->setOverscan(overscan_enabled->getState());
                         }
                     });
                     // screensaver time
                     auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
                     screensaver_time->setValue((float) (Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
                     s->addWithLabel("SCREENSAVER AFTER", screensaver_time);
                     s->addSaveFunc([screensaver_time] {
                         Settings::getInstance()->setInt("ScreenSaverTime",
                                                         (int) round(screensaver_time->getValue()) * (1000 * 60));
                     });

                     // screensaver behavior
                     auto screensaver_behavior = std::make_shared<OptionListComponent<std::string> >(mWindow,
                                                                                                     "TRANSITION STYLE",
                                                                                                     false);
                     std::vector<std::string> screensavers;
                     screensavers.push_back("dim");
                     screensavers.push_back("black");
                     for (auto it = screensavers.begin(); it != screensavers.end(); it++)
                         screensaver_behavior->add(*it, *it,
                                                   Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
                     s->addWithLabel("SCREENSAVER BEHAVIOR", screensaver_behavior);
                     s->addSaveFunc([screensaver_behavior] {
                         Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected());
                     });

                     // framerate
                     auto framerate = std::make_shared<SwitchComponent>(mWindow);
                     framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
                     s->addWithLabel("SHOW FRAMERATE", framerate);
                     s->addSaveFunc(
                             [framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });

                     // show help
                     auto show_help = std::make_shared<SwitchComponent>(mWindow);
                     show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
                     s->addWithLabel("ON-SCREEN HELP", show_help);
                     s->addSaveFunc(
                             [show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

                     // quick system select (left/right in game list view)
                     auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
                     quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
                     s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
                     s->addSaveFunc([quick_sys_select] {
                         Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState());
                     });

                     // transition style
                     auto transition_style = std::make_shared<OptionListComponent<std::string> >(mWindow,
                                                                                                 "TRANSITION STYLE", false);
                     std::vector<std::string> transitions;
                     transitions.push_back("fade");
                     transitions.push_back("slide");
                     for (auto it = transitions.begin(); it != transitions.end(); it++)
                         transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
                     s->addWithLabel("TRANSITION STYLE", transition_style);
                     s->addSaveFunc([transition_style] {
                         Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected());
                     });

                     // theme set
                     auto themeSets = ThemeData::getThemeSets();

                     if (!themeSets.empty()) {
                         auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
                         if (selectedSet == themeSets.end())
                             selectedSet = themeSets.begin();

                         auto theme_set = std::make_shared<OptionListComponent<std::string> >(mWindow, "THEME SET", false);
                         for (auto it = themeSets.begin(); it != themeSets.end(); it++)
                             theme_set->add(it->first, it->first, it == selectedSet);
                         s->addWithLabel("THEME SET", theme_set);

                         Window *window = mWindow;
                         s->addSaveFunc([window, theme_set] {
                             bool needReload = false;
                             if (Settings::getInstance()->getString("ThemeSet") != theme_set->getSelected())
                                 needReload = true;

                             Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

                             if (needReload)
                                 ViewController::get()->reloadAll(); // TODO - replace this with some sort of signal-based implementation
                         });
                     }

                     mWindow->pushGui(s);
                 });
    }

    addEntry("SOUND SETTINGS", 0x777777FF, true,
             [this] {
                 auto s = new GuiSettings(mWindow, "SOUND SETTINGS");

                 // volume
                 auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
                 volume->setValue((float) VolumeControl::getInstance()->getVolume());
                 s->addWithLabel("SYSTEM VOLUME", volume);

                 // disable sounds
                 auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
                 sounds_enabled->setState(!(RecalboxConf::getInstance()->get("audio.bgmusic") == "0"));
                 s->addWithLabel("FRONTEND MUSIC", sounds_enabled);

                 auto output_list = std::make_shared<OptionListComponent<std::string> >(mWindow, "OUTPUT DEVICE",
                                                                                        false);


                 std::string currentDevice = RecalboxConf::getInstance()->get("audio.device");
                 if (currentDevice.empty()) currentDevice = "auto";
                 output_list->add("HDMI", "hdmi", "hdmi" == currentDevice);
                 output_list->add("JACK", "jack", "jack" == currentDevice);
                 output_list->add("AUTO", "auto", "auto" == currentDevice);

                 if(RecalboxConf::getInstance()->get("system.es.menu") != "bartop"){
                    s->addWithLabel("OUTPUT DEVICE", output_list);
                 }
                 s->addSaveFunc([output_list, currentDevice,sounds_enabled,volume] {

                     VolumeControl::getInstance()->setVolume((int) round(volume->getValue()));
                     RecalboxConf::getInstance()->set("audio.volume",
                                                      std::to_string((int) round(volume->getValue())));

                     RecalboxConf::getInstance()->set("audio.bgmusic",
                                                                      sounds_enabled->getState() ? "1" : "0");
                     if (!sounds_enabled->getState())
                         AudioManager::getInstance()->stopMusic();
                     if (currentDevice != output_list->getSelected()) {
                         RecalboxConf::getInstance()->set("audio.device", output_list->getSelected());
                         RecalboxSystem::getInstance()->setAudioOutputDevice(output_list->getSelected());
                     }
                     RecalboxConf::getInstance()->saveRecalboxConf();
                 });

                 mWindow->pushGui(s);
             });

    if(RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
        addEntry("NETWORK SETTINGS", 0x777777FF, true,
                 [this] {
                     Window *window = mWindow;

                     auto s = new GuiSettings(mWindow, "NETWORK SETTINGS");
                     auto status = std::make_shared<TextComponent>(mWindow, RecalboxSystem::getInstance()->ping() ? "CONNECTED" : "NOT CONNECTED",
                                                                   Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                     s->addWithLabel("STATUS", status);
                     auto ip = std::make_shared<TextComponent>(mWindow, RecalboxSystem::getInstance()->getIpAdress(),
                                                               Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                     s->addWithLabel("IP ADDRESS", ip);
                     // Hostname
                     createInputTextRow(s, "HOSTNAME", "system.hostname", false);

                     // Wifi enable
                     auto enable_wifi = std::make_shared<SwitchComponent>(mWindow);
                     bool baseEnabled = RecalboxConf::getInstance()->get("wifi.enabled") == "1";
                     enable_wifi->setState(baseEnabled);
                     s->addWithLabel("ENABLE WIFI", enable_wifi);

                     // window, title, settingstring,
                     const std::string baseSSID = RecalboxConf::getInstance()->get("wifi.ssid");
                     createInputTextRow(s, "WIFI SSID", "wifi.ssid", false);
                     const std::string baseKEY = RecalboxConf::getInstance()->get("wifi.key");
                     createInputTextRow(s, "WIFI KEY", "wifi.key", true);

                     s->addSaveFunc([baseEnabled, baseSSID, baseKEY, enable_wifi, window] {
                         bool wifienabled = enable_wifi->getState();
                         RecalboxConf::getInstance()->set("wifi.enabled", wifienabled ? "1" : "0");
                         std::string newSSID = RecalboxConf::getInstance()->get("wifi.ssid");
                         std::string newKey = RecalboxConf::getInstance()->get("wifi.key");
                         RecalboxConf::getInstance()->saveRecalboxConf();
                         if (wifienabled) {
                             if (baseSSID != newSSID
                                 || baseKEY != newKey
                                 || !baseEnabled) {
                                 if (RecalboxSystem::getInstance()->enableWifi(newSSID, newKey)) {
                                     window->pushGui(
                                             new GuiMsgBox(window, "WIFI ENABLED")
                                     );
                                 } else {
                                     window->pushGui(
                                             new GuiMsgBox(window, "WIFI CONFIGURATION ERROR")
                                     );
                                 }
                             }
                         } else if (baseEnabled) {
                             RecalboxSystem::getInstance()->disableWifi();
                         }
                     });
                     mWindow->pushGui(s);


                 });
    }

    if(RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
        auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
        addEntry("SCRAPER", 0x777777FF, true,
                 [this, openScrapeNow] {
                     auto s = new GuiSettings(mWindow, "SCRAPER");

                     // scrape from
                     auto scraper_list = std::make_shared<OptionListComponent<std::string> >(mWindow, "SCRAPE FROM", false);
                     std::vector<std::string> scrapers = getScraperList();
                     for (auto it = scrapers.begin(); it != scrapers.end(); it++)
                         scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

                     s->addWithLabel("SCRAPE FROM", scraper_list);
                     s->addSaveFunc([scraper_list] {
                         Settings::getInstance()->setString("Scraper", scraper_list->getSelected());
                     });

                     // scrape ratings
                     auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
                     scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
                     s->addWithLabel("SCRAPE RATINGS", scrape_ratings);
                     s->addSaveFunc([scrape_ratings] {
                         Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState());
                     });

                     // scrape now
                     ComponentListRow row;
                     std::function<void()> openAndSave = openScrapeNow;
                     openAndSave = [s, openAndSave] {
                         s->save();
                         openAndSave();
                     };
                     row.makeAcceptInputHandler(openAndSave);

                     auto scrape_now = std::make_shared<TextComponent>(mWindow, "SCRAPE NOW", Font::get(FONT_SIZE_MEDIUM),
                                                                       0x777777FF);
                     auto bracket = makeArrow(mWindow);
                     row.addElement(scrape_now, true);
                     row.addElement(bracket, false);
                     s->addRow(row);

                     mWindow->pushGui(s);
                 });
    }

    addEntry("QUIT", 0x777777FF, true,
             [this] {
                 auto s = new GuiSettings(mWindow, "QUIT");

                 Window *window = mWindow;

                 ComponentListRow row;
                 row.makeAcceptInputHandler([window] {
                     window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES",
                                                   [] {
                                                       if (RecalboxSystem::getInstance()->reboot() != 0)  {
                                                           LOG(LogWarning) << "Restart terminated with non-zero result!";
                                                       }
                                                   }, "NO", nullptr));
                 });
                 row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM),
                                                                0x777777FF), true);
                 s->addRow(row);

                 row.elements.clear();
                 row.makeAcceptInputHandler([window] {
                     window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES",
                                                   [] {
                                                       if (RecalboxSystem::getInstance()->shutdown() != 0)  {
                                                           LOG(LogWarning) <<
                                                                           "Shutdown terminated with non-zero result!";
                                                       }
                                                   }, "NO", nullptr));
                 });
                 row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM),
                                                                0x777777FF), true);
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

void GuiMenu::createConfigInput() {

    GuiSettings *s = new GuiSettings(mWindow, "CONTROLLERS SETTINGS");

    Window *window = mWindow;

    ComponentListRow row;
    row.makeAcceptInputHandler([window, this, s] {
        window->pushGui(new GuiMsgBox(window, "I18NMESSAGECONTROLLERS", "OK",
                                      [window, this, s] {
                                          window->pushGui(new GuiDetectDevice(window, false, [this, s] {
                                              s->setSave(false);
                                              delete s;
                                              this->createConfigInput();
                                          }));
                                      }));
    });


    row.addElement(
            std::make_shared<TextComponent>(window, "CONFIGURE A CONTROLLER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
            true);
    s->addRow(row);


    row.elements.clear();

    std::function<void(void*)> showControllerList = [window, this, s](void * controllers){
        std::function<void(void*)> deletePairGui = [window](void * pairedPointer){
            bool paired = *((bool*)pairedPointer);
            window->pushGui(new GuiMsgBox(window, paired ? "CONTROLLER PAIRED" : "UNABLE TO PAIR CONTROLLER", "OK"));
        };
        if(controllers == NULL){
            window->pushGui(new GuiMsgBox(window, "AN ERROR OCCURED", "OK"));
        }else {
            std::vector<std::string>* resolvedControllers = ((std::vector<std::string>*)controllers);
            if(resolvedControllers->size() == 0){
                window->pushGui(new GuiMsgBox(window, "NO CONTROLLERS FOUND", "OK"));
            }else {
                GuiSettings * pairGui = new GuiSettings(window, "PAIR A BLUETOOTH CONTROLLER");

                for(std::vector<std::string>::iterator controllerString = ((std::vector<std::string>*)controllers)->begin(); controllerString != ((std::vector<std::string>*)controllers)->end(); ++controllerString) {

                    ComponentListRow controllerRow;
                    std::function<void()> pairController = [this,window,pairGui,controllerString,deletePairGui] {
                        window->pushGui(new GuiLoading(window, [controllerString] {
                            bool paired = RecalboxSystem::getInstance()->pairBluetooth(*controllerString);

                            return (void*)new bool(true);
                        }, deletePairGui));

                    };
                    controllerRow.makeAcceptInputHandler(pairController);
                    auto update = std::make_shared<TextComponent>(window, *controllerString,
                                                                  Font::get(FONT_SIZE_MEDIUM),
                                                                  0x777777FF);
                    auto bracket = makeArrow(window);
                    controllerRow.addElement(update, true);
                    controllerRow.addElement(bracket, false);
                    pairGui->addRow(controllerRow);
                }
                window->pushGui(pairGui);
            }
        }

    };

    row.makeAcceptInputHandler([window, this, s, showControllerList] {

        window->pushGui(new GuiLoading(window, [] {
            auto s =  RecalboxSystem::getInstance()->scanBluetooth();
            return (void *)s;
        }, showControllerList));
    });


    row.addElement(
            std::make_shared<TextComponent>(window, "PAIR A BLUETOOTH CONTROLLER", Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
            true);
    s->addRow(row);


    row.elements.clear();
    // quick system select (left/right in game list view)

    // Here we go; for each player
    std::list<int> alreadyTaken = std::list<int>();

    std::vector<std::shared_ptr<OptionListComponent<InputConfig*>>> options;
    for (int player = 0; player < 4; player++) {
        std::stringstream sstm;
        sstm << "INPUT P" << player + 1;
        std::string confName = sstm.str()+"NAME";
        std::string confGuid = sstm.str()+"GUID";

        LOG(LogInfo) <<player+1 <<" " << confName << " "<< confGuid;
        auto inputOptionList = std::make_shared<OptionListComponent<InputConfig*> >(mWindow, sstm.str(), false);
        options.push_back(inputOptionList);

        // Checking if a setting has been saved, else setting to default
        std::string configuratedName = Settings::getInstance()->getString(confName);
        std::string configuratedGuid = Settings::getInstance()->getString(confGuid);
        bool found = false;
        // For each available and configured input
        for (auto it = 0; it < InputManager::getInstance()->getNumJoysticks(); it++) {
            InputConfig *config = InputManager::getInstance()->getInputConfigByDevice(it);
            if (config->isConfigured()) {
                // create name
                std::stringstream dispNameSS;
                dispNameSS << "#" << config->getDeviceId() << " ";
                std::string deviceName = config->getDeviceName();
                if (deviceName.size() > 25) {
                    dispNameSS << deviceName.substr(0, 16) << "..." <<
                    deviceName.substr(deviceName.size() - 5, deviceName.size() - 1);
                } else {
                    dispNameSS << deviceName;
                }

                std::string displayName = dispNameSS.str();


                bool foundFromConfig = configuratedName == config->getDeviceName() && configuratedGuid == config->getDeviceGUIDString();
                int deviceID = config->getDeviceId();
                // Si la manette est configurée, qu'elle correspond a la configuration, et qu'elle n'est pas
                // deja selectionnée on l'ajoute en séléctionnée
                if (foundFromConfig
                    && std::find(alreadyTaken.begin(), alreadyTaken.end(), deviceID) == alreadyTaken.end()
                    && !found) {
                    found = true;
                    alreadyTaken.push_back(deviceID);
                    LOG(LogWarning) << "adding entry for player" << player << " (selected): " <<
                                    config->getDeviceName() << "  " << config->getDeviceGUIDString();
                    inputOptionList->add(displayName, config, true);
                } else {
                    LOG(LogWarning) << "adding entry for player" << player << " (not selected): " <<
                                    config->getDeviceName() << "  " << config->getDeviceGUIDString();
                    inputOptionList->add(displayName, config, false);
                }
            }
        }
        if (configuratedName.compare("") == 0 || !found) {
            LOG(LogWarning) << "adding default entry for player " << player << "(selected : true)";
            inputOptionList->add("DEFAULT", NULL, true);
        } else {
            LOG(LogWarning) << "adding default entry for player" << player << "(selected : false)";
            inputOptionList->add("DEFAULT", NULL, false);
        }

        // ADD default config

        // Populate controllers list
        s->addWithLabel(sstm.str(), inputOptionList);
    }
    s->addSaveFunc([this, options, window] {
        for (int player = 0; player < 4; player++) {
            std::stringstream sstm;
            sstm << "INPUT P" << player + 1;
            std::string confName = sstm.str()+"NAME";
            std::string confGuid = sstm.str()+"GUID";

            auto input_p1 = options.at(player);
            std::string name;
            std::string selectedName = input_p1->getSelectedName();

            if (selectedName.compare("DEFAULT") == 0) {
                name = "DEFAULT";
                Settings::getInstance()->setString(confName, name);
                Settings::getInstance()->setString(confGuid, "");
            } else {
                LOG(LogWarning) << "Found the selected controller ! : name in list  = " << selectedName;
                LOG(LogWarning) << "Found the selected controller ! : guid  = " << input_p1->getSelected()->getDeviceGUIDString();

                Settings::getInstance()->setString(confName, input_p1->getSelected()->getDeviceName());
                Settings::getInstance()->setString(confGuid, input_p1->getSelected()->getDeviceGUIDString());
            }
        }

        Settings::getInstance()->saveFile();

    });

    row.elements.clear();
    window->pushGui(s);

}

void GuiMenu::onSizeChanged() {
    mVersion.setSize(mSize.x(), 0);
    mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char *name, unsigned int color, bool add_arrow, const std::function<void()> &func) {
    std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);

    // populate the list
    ComponentListRow row;
    row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

    if (add_arrow) {
        std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
        row.addElement(bracket, false);
    }

    row.makeAcceptInputHandler(func);

    mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig *config, Input input) {
    if (GuiComponent::input(config, input))
        return true;

    if ((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0) {
        delete this;
        return true;
    }

    return false;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts() {
    std::vector<HelpPrompt> prompts;
    prompts.push_back(HelpPrompt("up/down", "choose"));
    prompts.push_back(HelpPrompt("a", "select"));
    prompts.push_back(HelpPrompt("start", "close"));
    return prompts;
}
