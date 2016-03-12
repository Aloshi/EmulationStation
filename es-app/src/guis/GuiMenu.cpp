#include <list>
#include <algorithm>
#include <string>
#include <sstream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

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

void GuiMenu::createInputTextRow(GuiSettings *gui, std::string title, const char *settingsID, bool password) {
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

GuiMenu::GuiMenu(Window *window) : GuiComponent(window), mMenu(window, _("MAIN MENU").c_str()), mVersion(window) {
    // MAIN MENU

    // KODI >
    // ROM MANAGER >
    // SYSTEM >
    // GAMES >
    // CONTROLLERS >
    // UI SETTINGS >
    // SOUND SETTINGS >
    // NETWORK >
    // SCRAPER >
    // QUIT >
    if (RecalboxConf::getInstance()->get("kodi.enabled") == "1") {
      addEntry(_("KODI MEDIA CENTER").c_str(), 0x777777FF, true,
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
    if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
      addEntry(_("SYSTEM SETTINGS").c_str(), 0x777777FF, true,
                 [this] {
                     Window *window = mWindow;

                     auto s = new GuiSettings(mWindow, _("SYSTEM SETTINGS").c_str());

                     auto version = std::make_shared<TextComponent>(mWindow,
                                                                    RecalboxSystem::getInstance()->getVersion(),
                                                                    Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                     s->addWithLabel(_("VERSION"), version);
                     bool warning = RecalboxSystem::getInstance()->isFreeSpaceLimit();
                     auto space = std::make_shared<TextComponent>(mWindow,
                                                                  RecalboxSystem::getInstance()->getFreeSpaceInfo(),
                                                                  Font::get(FONT_SIZE_MEDIUM),
                                                                  warning ? 0xFF0000FF : 0x777777FF);
                     s->addWithLabel(_("DISK USAGE"), space);

                     std::vector<std::string> availableStorage = RecalboxSystem::getInstance()->getAvailableStorageDevices();
                     std::string selectedStorage = RecalboxSystem::getInstance()->getCurrentStorage();

                     // Storage device
                     auto optionsStorage = std::make_shared<OptionListComponent<std::string> >(window, _("STORAGE DEVICE"),
                                                                                                false);
                     for(auto it = availableStorage.begin(); it != availableStorage.end(); it++){
                         if((*it) != "RAM"){
                             if (boost::starts_with((*it), "DEV")){
                                 std::vector<std::string> tokens;
                                 boost::split( tokens, (*it), boost::is_any_of(" ") );
                                 if(tokens.size()>= 3){
                                     optionsStorage->add(tokens.at(2), (*it), selectedStorage == std::string("DEV "+tokens.at(1)));
                                 }
                             } else {
                                 optionsStorage->add((*it), (*it), selectedStorage == (*it));
                             }
                         }
                     }
                     s->addWithLabel(_("STORAGE DEVICE"), optionsStorage);


                     // language choice
                     auto language_choice = std::make_shared<OptionListComponent<std::string> >(window, _("LANGUAGE"),
                                                                                                false);
                     std::string language = RecalboxConf::getInstance()->get("system.language");
                     if (language.empty()) language = "en_US";
		     language_choice->add("BASQUE",    "eu_ES", language == "eu_ES");
		     language_choice->add("CHINESE",   "zh_CN", language == "zh_CN");
		     language_choice->add("DEUTSCH",   "de_DE", language == "de_DE");
		     language_choice->add("ENGLISH",   "en_US", language == "en_US");
                     language_choice->add("ESPAÑOL",   "es_ES", language == "es_ES");
		     language_choice->add("FRANÇAIS",  "fr_FR", language == "fr_FR");
		     language_choice->add("ITALIANO",  "it_IT", language == "it_IT");
                     language_choice->add("PORTUGUES", "pt_BR", language == "pt_BR");
		     language_choice->add("SVENSKA",   "sv_SE", language == "sv_SE");
                     language_choice->add("TÜRKÇE",    "tr_TR", language == "tr_TR");

                     s->addWithLabel(_("LANGUAGE"), language_choice);

                     // Overclock choice
                     auto overclock_choice = std::make_shared<OptionListComponent<std::string> >(window, _("OVERCLOCK"),
                                                                                                 false);
#ifdef RPI_VERSION
#if RPI_VERSION == 1
                     std::string currentOverclock = Settings::getInstance()->getString("Overclock");
                     overclock_choice->add(_("EXTREM (1100Mhz)"), "extrem", currentOverclock == "extrem");
                     overclock_choice->add(_("TURBO (1000Mhz)"), "turbo", currentOverclock == "turbo");
                     overclock_choice->add(_("HIGH (950Mhz)"), "high", currentOverclock == "high");
                     overclock_choice->add(_("NONE (700Mhz)"), "none", currentOverclock == "none");
#elif RPI_VERSION == 2
                     std::string currentOverclock = Settings::getInstance()->getString("Overclock");
                     overclock_choice->add(_("EXTREM (1100Mhz)"), "rpi2-extrem", currentOverclock == "rpi2-extrem");
                     overclock_choice->add(_("TURBO (1050Mhz)+"), "rpi2-turbo", currentOverclock == "rpi2-turbo");
                     overclock_choice->add(_("HIGH (1050Mhz)"), "rpi2-high", currentOverclock == "rpi2-high");
                     overclock_choice->add(_("NONE (900Mhz)"), "none", currentOverclock == "none");
#elif RPI_VERSION == 3
                     overclock_choice->add(_("NONE (1200Mhz)"), "none", true);
#endif
#else
                     overclock_choice->add(_("NONE"), "none", true);
#endif
                     s->addWithLabel(_("OVERCLOCK"), overclock_choice);

                     // Updates
                     {
                         ComponentListRow row;
                         std::function<void()> openGuiD = [this] {
			   GuiSettings *updateGui = new GuiSettings(mWindow, _("UPDATES").c_str());
                             // Enable updates
                             auto updates_enabled = std::make_shared<SwitchComponent>(mWindow);
                             updates_enabled->setState(
                                     RecalboxConf::getInstance()->get("updates.enabled") == "1");
			   updateGui->addWithLabel(_("AUTO UPDATES"), updates_enabled);

                             // Start update
                             {
                                 ComponentListRow updateRow;
                                 std::function<void()> openGui = [this] { mWindow->pushGui(new GuiUpdate(mWindow)); };
                                 updateRow.makeAcceptInputHandler(openGui);
                                 auto update = std::make_shared<TextComponent>(mWindow, _("START UPDATE"),
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
                         auto update = std::make_shared<TextComponent>(mWindow, _("UPDATES"), Font::get(FONT_SIZE_MEDIUM),
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
			   GuiSettings *kodiGui = new GuiSettings(mWindow, _("KODI SETTINGS").c_str());
                             auto kodiEnabled = std::make_shared<SwitchComponent>(mWindow);
                             kodiEnabled->setState(RecalboxConf::getInstance()->get("kodi.enabled") == "1");
			   kodiGui->addWithLabel(_("ENABLE KODI"), kodiEnabled);
                             auto kodiAtStart = std::make_shared<SwitchComponent>(mWindow);
                             kodiAtStart->setState(
                                     RecalboxConf::getInstance()->get("kodi.atstartup") == "1");
			   kodiGui->addWithLabel(_("KODI AT START"), kodiAtStart);
                             auto kodiX = std::make_shared<SwitchComponent>(mWindow);
                             kodiX->setState(RecalboxConf::getInstance()->get("kodi.xbutton") == "1");
			   kodiGui->addWithLabel(_("START KODI WITH X"), kodiX);
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
                         auto kodiSettings = std::make_shared<TextComponent>(mWindow, _("KODI SETTINGS"),
                                                                             Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                         auto bracket = makeArrow(mWindow);
                         row.addElement(kodiSettings, true);
                         row.addElement(bracket, false);
                         s->addRow(row);
                     }


                     s->addSaveFunc([overclock_choice, window, language_choice, language, optionsStorage, selectedStorage] {
                         bool reboot = false;
                         if (optionsStorage->changed()) {
                             RecalboxSystem::getInstance()->setStorage(optionsStorage->getSelected());
                             reboot = true;
                         }

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
					     new GuiMsgBox(window, _("THE SYSTEM WILL NOW REBOOT"), _("OK"),
                                           [window] {
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

    addEntry(_("GAMES SETTINGS").c_str(), 0x777777FF, true,
             [this] {
                 auto s = new GuiSettings(mWindow, _("GAMES SETTINGS").c_str());
                 if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {

                     // Screen ratio choice
                     auto ratio_choice = createRatioOptionList(mWindow, "global");
                     s->addWithLabel(_("GAME RATIO"), ratio_choice);
                     s->addSaveFunc([ratio_choice] {
                         RecalboxConf::getInstance()->set("global.ratio", ratio_choice->getSelected());
                         RecalboxConf::getInstance()->saveRecalboxConf();
                     });
                 }
                 // smoothing
                 auto smoothing_enabled = std::make_shared<SwitchComponent>(mWindow);
                 smoothing_enabled->setState(RecalboxConf::getInstance()->get("global.smooth") == "1");
                 s->addWithLabel(_("SMOOTH GAMES"), smoothing_enabled);

                 // rewind
                 auto rewind_enabled = std::make_shared<SwitchComponent>(mWindow);
                 rewind_enabled->setState(RecalboxConf::getInstance()->get("global.rewind") == "1");
                 s->addWithLabel(_("REWIND"), rewind_enabled);

                 // autosave/load
                 auto autosave_enabled = std::make_shared<SwitchComponent>(mWindow);
                 autosave_enabled->setState(RecalboxConf::getInstance()->get("global.autosave") == "1");
                 s->addWithLabel(_("AUTO SAVE/LOAD"), autosave_enabled);

                 // Shaders preset

                 auto shaders_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("SHADERS SET"),
                                                                                            false);
                 std::string currentShader = RecalboxConf::getInstance()->get("global.shaderset");
                 if (currentShader.empty()) {
                     currentShader = std::string("none");
                 }

                 shaders_choices->add(_("NONE"), "none", currentShader == "none");
                 shaders_choices->add(_("SCANLINES"), "scanlines", currentShader == "scanlines");
                 shaders_choices->add(_("RETRO"), "retro", currentShader == "retro");
                 s->addWithLabel(_("SHADERS SET"), shaders_choices);

                 if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {

                     // Retroachievements
                     {
                         ComponentListRow row;
                         std::function<void()> openGui = [this] {
                             GuiSettings *retroachievements = new GuiSettings(mWindow,
                                                                              _("RETROACHIEVEMENTS SETTINGS").c_str());
                             // retroachievements_enable
                             auto retroachievements_enabled = std::make_shared<SwitchComponent>(mWindow);
                             retroachievements_enabled->setState(
                                     RecalboxConf::getInstance()->get("global.retroachievements") == "1");
                             retroachievements->addWithLabel(_("RETROACHIEVEMENTS"), retroachievements_enabled);

                             // retroachievements, username, password
                             createInputTextRow(retroachievements, _("USERNAME"), "global.retroachievements.username",
                                                false);
                             createInputTextRow(retroachievements, _("PASSWORD"), "global.retroachievements.password",
                                                false);


                             retroachievements->addSaveFunc([retroachievements_enabled] {
                                 RecalboxConf::getInstance()->set("global.retroachievements",
                                                                  retroachievements_enabled->getState() ? "1" : "0");
                                 RecalboxConf::getInstance()->saveRecalboxConf();
                             });
                             mWindow->pushGui(retroachievements);
                         };
                         row.makeAcceptInputHandler(openGui);
                         auto retroachievementsSettings = std::make_shared<TextComponent>(mWindow,
                                                                                          _("RETROACHIEVEMENTS SETTINGS"),
                                                                                          Font::get(FONT_SIZE_MEDIUM),
                                                                                          0x777777FF);
                         auto bracket = makeArrow(mWindow);
                         row.addElement(retroachievementsSettings, true);
                         row.addElement(bracket, false);
                         s->addRow(row);
                     }
                 }
                 // Custom config for systems
                 if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
                     ComponentListRow row;
                     std::function<void()> openGuiD = [this, s] {
                         s->save();
                         GuiSettings *configuration = new GuiSettings(mWindow, _("ADVANCED").c_str());
                         // For each activated system
                         std::vector<SystemData *> systems = SystemData::sSystemVector;
                         for (auto system = systems.begin(); system != systems.end(); system++) {
                             if ((*system) != SystemData::getFavoriteSystem()) {
                                 ComponentListRow systemRow;
                                 auto systemText = std::make_shared<TextComponent>(mWindow, (*system)->getFullName(),
                                                                                   Font::get(FONT_SIZE_MEDIUM),
                                                                                   0x777777FF);
                                 auto bracket = makeArrow(mWindow);
                                 systemRow.addElement(systemText, true);
                                 systemRow.addElement(bracket, false);
                                 SystemData *systemData = (*system);
                                 systemRow.makeAcceptInputHandler([this, systemData] {
                                     popSystemConfigurationGui(systemData, "");
                                 });
                                 configuration->addRow(systemRow);
                             }
                         }
                         mWindow->pushGui(configuration);

                     };
                     // Advanced button
                     row.makeAcceptInputHandler(openGuiD);
                     auto advanced = std::make_shared<TextComponent>(mWindow, _("ADVANCED"), Font::get(FONT_SIZE_MEDIUM),
                                                                     0x777777FF);
                     auto bracket = makeArrow(mWindow);
                     row.addElement(advanced, true);
                     row.addElement(bracket, false);
                     s->addRow(row);
                 }
                 s->addSaveFunc([smoothing_enabled, rewind_enabled, shaders_choices, autosave_enabled] {
                     RecalboxConf::getInstance()->set("global.smooth", smoothing_enabled->getState() ? "1" : "0");
                     RecalboxConf::getInstance()->set("global.rewind", rewind_enabled->getState() ? "1" : "0");
                     RecalboxConf::getInstance()->set("global.shaderset", shaders_choices->getSelected());
                     RecalboxConf::getInstance()->set("global.autosave", autosave_enabled->getState() ? "1" : "0");
                     RecalboxConf::getInstance()->saveRecalboxConf();
                 });
                 // reread game list
                 if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {

                     ComponentListRow row;
                     Window *window = mWindow;

                     row.makeAcceptInputHandler([this, window] {
                         window->pushGui(new GuiMsgBox(window, _("REALLY UPDATE GAMES LISTS ?"), _("YES"),
                                                       [this, window] {
                                                           ViewController::get()->goToStart();
                                                           window->renderShutdownScreen();
                                                           delete ViewController::get();
                                                           SystemData::deleteSystems();
                                                           SystemData::loadConfig();
                                                           GuiComponent *gui;
                                                           while ((gui = window->peekGui()) != NULL) {
                                                               window->removeGui(gui);
                                                               delete gui;
                                                           }
                                                           ViewController::init(window);
                                                           ViewController::get()->reloadAll();
                                                           window->pushGui(ViewController::get());
                                                       }, _("NO"), nullptr));
                     });
                     row.addElement(
                             std::make_shared<TextComponent>(window, _("UPDATE GAMES LISTS"),
                                                             Font::get(FONT_SIZE_MEDIUM),
                                                             0x777777FF), true);
                     s->addRow(row);
                 }
                 mWindow->pushGui(s);
             }

    );

    if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
      addEntry(_("CONTROLLERS SETTINGS").c_str(), 0x777777FF, true, [this] { this->createConfigInput(); });
    }
    if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {

      addEntry(_("UI SETTINGS").c_str(), 0x777777FF, true,
                 [this] {
		 auto s = new GuiSettings(mWindow, _("UI SETTINGS").c_str());

                     // overscan
                     auto overscan_enabled = std::make_shared<SwitchComponent>(mWindow);
                     overscan_enabled->setState(Settings::getInstance()->getBool("Overscan"));
                     s->addWithLabel(_("OVERSCAN"), overscan_enabled);
                     s->addSaveFunc([overscan_enabled] {
                         if (Settings::getInstance()->getBool("Overscan") != overscan_enabled->getState()) {
                             Settings::getInstance()->setBool("Overscan", overscan_enabled->getState());
                             RecalboxSystem::getInstance()->setOverscan(overscan_enabled->getState());
                         }
                     });
                     // screensaver time
                     auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
                     screensaver_time->setValue(
                             (float) (Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
                     s->addWithLabel(_("SCREENSAVER AFTER"), screensaver_time);
                     s->addSaveFunc([screensaver_time] {
                         Settings::getInstance()->setInt("ScreenSaverTime",
                                                         (int) round(screensaver_time->getValue()) * (1000 * 60));
                     });

                     // screensaver behavior
                     auto screensaver_behavior = std::make_shared<OptionListComponent<std::string> >(mWindow,
                                                                                                     _("TRANSITION STYLE"),
                                                                                                     false);
                     std::vector<std::string> screensavers;
                     screensavers.push_back("dim");
                     screensavers.push_back("black");
                     for (auto it = screensavers.begin(); it != screensavers.end(); it++)
                         screensaver_behavior->add(*it, *it,
                                                   Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
                     s->addWithLabel(_("SCREENSAVER BEHAVIOR"), screensaver_behavior);
                     s->addSaveFunc([screensaver_behavior] {
                         Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected());
                     });

                     // framerate
                     auto framerate = std::make_shared<SwitchComponent>(mWindow);
                     framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
                     s->addWithLabel(_("SHOW FRAMERATE"), framerate);
                     s->addSaveFunc(
                             [framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });

                     // show help
                     auto show_help = std::make_shared<SwitchComponent>(mWindow);
                     show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
                     s->addWithLabel(_("ON-SCREEN HELP"), show_help);
                     s->addSaveFunc(
                             [show_help] {
                                 Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState());
                             });

                     // quick system select (left/right in game list view)
                     auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
                     quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
                     s->addWithLabel(_("QUICK SYSTEM SELECT"), quick_sys_select);
                     s->addSaveFunc([quick_sys_select] {
                         Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState());
                     });

                     // transition style
                     auto transition_style = std::make_shared<OptionListComponent<std::string> >(mWindow,
                                                                                                 _("TRANSITION STYLE"),
                                                                                                 false);
                     std::vector<std::string> transitions;
                     transitions.push_back("fade");
                     transitions.push_back("slide");
                     for (auto it = transitions.begin(); it != transitions.end(); it++)
                         transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
                     s->addWithLabel(_("TRANSITION STYLE"), transition_style);
                     s->addSaveFunc([transition_style] {
                         Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected());
                     });

                     // theme set
                     auto themeSets = ThemeData::getThemeSets();

                     if (!themeSets.empty()) {
                         auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
                         if (selectedSet == themeSets.end())
                             selectedSet = themeSets.begin();

                         auto theme_set = std::make_shared<OptionListComponent<std::string> >(mWindow, _("THEME SET"),
                                                                                              false);
                         for (auto it = themeSets.begin(); it != themeSets.end(); it++)
                             theme_set->add(it->first, it->first, it == selectedSet);
                         s->addWithLabel(_("THEME SET"), theme_set);

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

    addEntry(_("SOUND SETTINGS").c_str(), 0x777777FF, true,
             [this] {
	       auto s = new GuiSettings(mWindow, _("SOUND SETTINGS").c_str());

                 // volume
                 auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
                 volume->setValue((float) VolumeControl::getInstance()->getVolume());
                 s->addWithLabel(_("SYSTEM VOLUME"), volume);

                 // disable sounds
                 auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
                 sounds_enabled->setState(!(RecalboxConf::getInstance()->get("audio.bgmusic") == "0"));
                 s->addWithLabel(_("FRONTEND MUSIC"), sounds_enabled);

                 auto output_list = std::make_shared<OptionListComponent<std::string> >(mWindow, _("OUTPUT DEVICE"),
                                                                                        false);


                 std::string currentDevice = RecalboxConf::getInstance()->get("audio.device");
                 if (currentDevice.empty()) currentDevice = "auto";
                 output_list->add(_("HDMI"), "hdmi", "hdmi" == currentDevice);
                 output_list->add(_("JACK"), "jack", "jack" == currentDevice);
                 output_list->add(_("AUTO"), "auto", "auto" == currentDevice);

                 if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
		   s->addWithLabel(_("OUTPUT DEVICE"), output_list);
                 }
                 s->addSaveFunc([output_list, currentDevice, sounds_enabled, volume] {

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

    if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
      addEntry(_("NETWORK SETTINGS").c_str(), 0x777777FF, true,
                 [this] {
                     Window *window = mWindow;

                     auto s = new GuiSettings(mWindow, _("NETWORK SETTINGS").c_str());
                     auto status = std::make_shared<TextComponent>(mWindow,
                                                                   RecalboxSystem::getInstance()->ping() ? _("CONNECTED")
								   : _("NOT CONNECTED"),
                                                                   Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                     s->addWithLabel(_("STATUS"), status);
                     auto ip = std::make_shared<TextComponent>(mWindow, RecalboxSystem::getInstance()->getIpAdress(),
                                                               Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
                     s->addWithLabel(_("IP ADDRESS"), ip);
                     // Hostname
                     createInputTextRow(s, _("HOSTNAME"), "system.hostname", false);

                     // Wifi enable
                     auto enable_wifi = std::make_shared<SwitchComponent>(mWindow);
                     bool baseEnabled = RecalboxConf::getInstance()->get("wifi.enabled") == "1";
                     enable_wifi->setState(baseEnabled);
                     s->addWithLabel(_("ENABLE WIFI"), enable_wifi);

                     // window, title, settingstring,
                     const std::string baseSSID = RecalboxConf::getInstance()->get("wifi.ssid");
                     createInputTextRow(s, _("WIFI SSID"), "wifi.ssid", false);
                     const std::string baseKEY = RecalboxConf::getInstance()->get("wifi.key");
                     createInputTextRow(s, _("WIFI KEY"), "wifi.key", true);

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
						     new GuiMsgBox(window, _("WIFI ENABLED"))
                                     );
                                 } else {
                                     window->pushGui(
						     new GuiMsgBox(window, _("WIFI CONFIGURATION ERROR"))
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

    if (RecalboxConf::getInstance()->get("system.es.menu") != "bartop") {
        auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
        addEntry(_("SCRAPER").c_str(), 0x777777FF, true,
                 [this, openScrapeNow] {
		   auto s = new GuiSettings(mWindow, _("SCRAPER").c_str());

                     // scrape from
		   auto scraper_list = std::make_shared<OptionListComponent<std::string> >(mWindow, _("SCRAPE FROM"),
                                                                                             false);
                     std::vector<std::string> scrapers = getScraperList();
                     for (auto it = scrapers.begin(); it != scrapers.end(); it++)
                         scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

                     s->addWithLabel(_("SCRAPE FROM"), scraper_list);
                     s->addSaveFunc([scraper_list] {
                         Settings::getInstance()->setString("Scraper", scraper_list->getSelected());
                     });

                     // scrape ratings
                     auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
                     scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
                     s->addWithLabel(_("SCRAPE RATINGS"), scrape_ratings);
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

                     auto scrape_now = std::make_shared<TextComponent>(mWindow, _("SCRAPE NOW"),
                                                                       Font::get(FONT_SIZE_MEDIUM),
                                                                       0x777777FF);
                     auto bracket = makeArrow(mWindow);
                     row.addElement(scrape_now, true);
                     row.addElement(bracket, false);
                     s->addRow(row);

                     mWindow->pushGui(s);
                 });
    }

    addEntry(_("QUIT").c_str(), 0x777777FF, true,
             [this] {
	       auto s = new GuiSettings(mWindow, _("QUIT").c_str());

                 Window *window = mWindow;

                 ComponentListRow row;
                 row.makeAcceptInputHandler([window] {
                     window->pushGui(new GuiMsgBox(window, _("REALLY RESTART?"), _("YES"),
                                                   [] {
                                                       if (RecalboxSystem::getInstance()->reboot() != 0) {
                                                           LOG(LogWarning) <<
                                                                           "Restart terminated with non-zero result!";
                                                       }
                                                   }, _("NO"), nullptr));
                 });
                 row.addElement(std::make_shared<TextComponent>(window, _("RESTART SYSTEM"), Font::get(FONT_SIZE_MEDIUM),
                                                                0x777777FF), true);
                 s->addRow(row);

                 row.elements.clear();
                 row.makeAcceptInputHandler([window] {
                     window->pushGui(new GuiMsgBox(window, _("REALLY SHUTDOWN?"), _("YES"),
                                                   [] {
                                                       if (RecalboxSystem::getInstance()->shutdown() != 0) {
                                                           LOG(LogWarning) <<
                                                                           "Shutdown terminated with non-zero result!";
                                                       }
                                                   }, _("NO"), nullptr));
                 });
                 row.addElement(std::make_shared<TextComponent>(window, _("SHUTDOWN SYSTEM"), Font::get(FONT_SIZE_MEDIUM),
                                                                0x777777FF), true);
                 s->addRow(row);

                 /*if(Settings::getInstance()->getBool("ShowExit"))
                 {
                     row.elements.clear();
                     row.makeAcceptInputHandler([window] {
                         window->pushGui(new GuiMsgBox(window, _("REALLY QUIT?"), _("YES"),
                         [] {
                             SDL_Event ev;
                             ev.type = SDL_QUIT;
                             SDL_PushEvent(&ev);
                         }, _("NO"), nullptr));
                     });
                     row.addElement(std::make_shared<TextComponent>(window, _("QUIT EMULATIONSTATION"), Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
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

GuiMenu::~GuiMenu() {
  clearLoadedInput();
}

void GuiMenu::popSystemConfigurationGui(SystemData *systemData, std::string previouslySelectedEmulator) const {
    // The system configuration
    GuiSettings *systemConfiguration = new GuiSettings(mWindow,
                                                       systemData->getFullName().c_str());
    //Emulator choice
    auto emu_choice = std::make_shared<OptionListComponent<std::string>>(mWindow, "emulator", false);
    bool selected = false;
    std::string selectedEmulator = "";
    for (auto it = systemData->getEmulators()->begin(); it != systemData->getEmulators()->end(); it++) {
        bool found;
        std::string curEmulatorName = it->first;
        if (previouslySelectedEmulator != "") {
            // We just changed the emulator
            found = previouslySelectedEmulator == curEmulatorName;
        } else {
            found = (RecalboxConf::getInstance()->get(systemData->getName() + ".emulator") == curEmulatorName);
        }
        if (found) {
            selectedEmulator = curEmulatorName;
        }
        selected = selected || found;
        emu_choice->add(curEmulatorName, curEmulatorName, found);
    }
    emu_choice->add("default", "default", !selected);
    emu_choice->setSelectedChangedCallback([this, systemConfiguration, systemData](std::string s) {
        popSystemConfigurationGui(systemData, s);
        delete systemConfiguration;
    });
    systemConfiguration->addWithLabel(_("Emulator"), emu_choice);

    // Core choice
    auto core_choice = std::make_shared<OptionListComponent<std::string> >(mWindow, _("Core"), false);
    selected = false;
    for (auto emulator = systemData->getEmulators()->begin();
         emulator != systemData->getEmulators()->end(); emulator++) {
        if (selectedEmulator == emulator->first) {
            for (auto core = emulator->second->begin(); core != emulator->second->end(); core++) {
                bool found = (RecalboxConf::getInstance()->get(systemData->getName() + ".core") == *core);
                selected = selected || found;
                core_choice->add(*core, *core, found);
            }
        }
    }
    core_choice->add("default", "default", !selected);
    systemConfiguration->addWithLabel(_("Core"), core_choice);


    // Screen ratio choice
    auto ratio_choice = createRatioOptionList(mWindow, systemData->getName());
    systemConfiguration->addWithLabel(_("GAME RATIO"), ratio_choice);
    // smoothing
    auto smoothing_enabled = std::make_shared<SwitchComponent>(mWindow);
    smoothing_enabled->setState(RecalboxConf::getInstance()->get(systemData->getName() + ".smooth", RecalboxConf::getInstance()->get("global.smooth")) == "1");
    systemConfiguration->addWithLabel(_("SMOOTH GAMES"), smoothing_enabled);
    // rewind
    auto rewind_enabled = std::make_shared<SwitchComponent>(mWindow);
    rewind_enabled->setState(
            RecalboxConf::getInstance()->get(systemData->getName() + ".rewind", RecalboxConf::getInstance()->get("global.rewind")) == "1");
    systemConfiguration->addWithLabel(_("REWIND"), rewind_enabled);
    // autosave
    auto autosave_enabled = std::make_shared<SwitchComponent>(mWindow);
    autosave_enabled->setState(
            RecalboxConf::getInstance()->get(systemData->getName() + ".autosave", RecalboxConf::getInstance()->get("global.autosave")) == "1");
    systemConfiguration->addWithLabel(_("AUTO SAVE/LOAD"), autosave_enabled);


    systemConfiguration->addSaveFunc(
            [systemData, smoothing_enabled, rewind_enabled, ratio_choice, emu_choice, core_choice, autosave_enabled] {
                if(ratio_choice->changed()){
                    RecalboxConf::getInstance()->set(systemData->getName() + ".ratio",
                                                     ratio_choice->getSelected());
                }
                if(rewind_enabled->changed()) {
                    RecalboxConf::getInstance()->set(systemData->getName() + ".rewind",
                                                     rewind_enabled->getState() ? "1" : "0");
                }
                if(smoothing_enabled->changed()){
                    RecalboxConf::getInstance()->set(systemData->getName() + ".smooth",
                                                     smoothing_enabled->getState() ? "1" : "0");
                }
                if(emu_choice->changed()) {
                    RecalboxConf::getInstance()->set(systemData->getName() + ".emulator", emu_choice->getSelected());
                }
                if(core_choice->changed()){
                    RecalboxConf::getInstance()->set(systemData->getName() + ".core", core_choice->getSelected());
                }
                if(autosave_enabled->changed()) {
                    RecalboxConf::getInstance()->set(systemData->getName() + ".autosave",
                                                     autosave_enabled->getState() ? "1" : "0");
                }
                RecalboxConf::getInstance()->saveRecalboxConf();
            });
    mWindow->pushGui(systemConfiguration);
}

void GuiMenu::createConfigInput() {

  GuiSettings *s = new GuiSettings(mWindow, _("CONTROLLERS SETTINGS").c_str());

    Window *window = mWindow;

    ComponentListRow row;
    row.makeAcceptInputHandler([window, this, s] {
        window->pushGui(new GuiMsgBox(window,
				      _("YOU ARE GOING TO CONFIGURE A CONTROLLER. IF YOU HAVE ONLY ONE JOYSTICK, "
				      "CONFIGURE THE DIRECTIONS KEYS AND SKIP JOYSTICK CONFIG BY HOLDING A BUTTON. "
				      "IF YOU DO NOT HAVE A SPECIAL KEY FOR HOTKEY, CHOOSE THE SELECT BUTTON. SKIP "
				      "ALL BUTTONS YOU DO NOT HAVE BY HOLDING A KEY. BUTTONS NAMES ARE BASED ON THE "
					"SNES CONTROLLER."), _("OK"),
                                      [window, this, s] {
                                          window->pushGui(new GuiDetectDevice(window, false, [this, s] {
                                              s->setSave(false);
                                              delete s;
                                              this->createConfigInput();
                                          }));
                                      }));
    });


    row.addElement(
		   std::make_shared<TextComponent>(window, _("CONFIGURE A CONTROLLER"), Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
            true);
    s->addRow(row);

    row.elements.clear();

    std::function<void(void *)> showControllerList = [window, this, s](void *controllers) {
        std::function<void(void *)> deletePairGui = [window](void *pairedPointer) {
            bool paired = *((bool *) pairedPointer);
            window->pushGui(new GuiMsgBox(window, paired ? _("CONTROLLER PAIRED") : _("UNABLE TO PAIR CONTROLLER"), _("OK")));
        };
        if (controllers == NULL) {
	  window->pushGui(new GuiMsgBox(window, _("AN ERROR OCCURED"), _("OK")));
        } else {
            std::vector<std::string> *resolvedControllers = ((std::vector<std::string> *) controllers);
            if (resolvedControllers->size() == 0) {
	      window->pushGui(new GuiMsgBox(window, _("NO CONTROLLERS FOUND"), _("OK")));
            } else {
	            GuiSettings *pairGui = new GuiSettings(window, _("PAIR A BLUETOOTH CONTROLLER").c_str());
                for (std::vector<std::string>::iterator controllerString = ((std::vector<std::string> *) controllers)->begin();
                     controllerString != ((std::vector<std::string> *) controllers)->end(); ++controllerString) {

                    ComponentListRow controllerRow;
                    std::function<void()> pairController = [this, window, pairGui, controllerString, deletePairGui] {
                        window->pushGui(new GuiLoading(window, [controllerString] {
                            bool paired = RecalboxSystem::getInstance()->pairBluetooth(*controllerString);

                            return (void *) new bool(true);
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
            auto s = RecalboxSystem::getInstance()->scanBluetooth();
            return (void *) s;
        }, showControllerList));
    });


    row.addElement(
		   std::make_shared<TextComponent>(window, _("PAIR A BLUETOOTH CONTROLLER"), Font::get(FONT_SIZE_MEDIUM),
                                            0x777777FF),
            true);
    s->addRow(row);
    row.elements.clear();

    row.makeAcceptInputHandler([window, this, s] {
        RecalboxSystem::getInstance()->forgetBluetoothControllers();
        window->pushGui(new GuiMsgBox(window,
                                      _("CONTROLLERS LINKS HAVE BEEN DELETED."), _("OK")));
    });
    row.addElement(
            std::make_shared<TextComponent>(window, _("FORGET BLUETOOTH CONTROLLERS"), Font::get(FONT_SIZE_MEDIUM),
                                            0x777777FF),
            true);
    s->addRow(row);
    row.elements.clear();



    row.elements.clear();

    // Here we go; for each player
    std::list<int> alreadyTaken = std::list<int>();

    // clear the current loaded inputs
    clearLoadedInput();

    std::vector<std::shared_ptr<OptionListComponent<StrInputConfig *>>> options;
    char strbuf[256];

    for (int player = 0; player < MAX_PLAYERS; player++) {
        std::stringstream sstm;
        sstm << "INPUT P" << player + 1;
        std::string confName = sstm.str() + "NAME";
        std::string confGuid = sstm.str() + "GUID";
	snprintf(strbuf, 256, _("INPUT P%i").c_str(), player+1);

        LOG(LogInfo) << player + 1 << " " << confName << " " << confGuid;
        auto inputOptionList = std::make_shared<OptionListComponent<StrInputConfig *> >(mWindow, strbuf, false);
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


                bool foundFromConfig = configuratedName == config->getDeviceName() &&
                                       configuratedGuid == config->getDeviceGUIDString();
                int deviceID = config->getDeviceId();
                // Si la manette est configurée, qu'elle correspond a la configuration, et qu'elle n'est pas
                // deja selectionnée on l'ajoute en séléctionnée
		StrInputConfig* newInputConfig = new StrInputConfig(config->getDeviceName(), config->getDeviceGUIDString());
		mLoadedInput.push_back(newInputConfig);

                if (foundFromConfig
                    && std::find(alreadyTaken.begin(), alreadyTaken.end(), deviceID) == alreadyTaken.end()
                    && !found) {
                    found = true;
                    alreadyTaken.push_back(deviceID);
                    LOG(LogWarning) << "adding entry for player" << player << " (selected): " <<
                                    config->getDeviceName() << "  " << config->getDeviceGUIDString();
                    inputOptionList->add(displayName, newInputConfig, true);
                } else {
                    LOG(LogWarning) << "adding entry for player" << player << " (not selected): " <<
                                    config->getDeviceName() << "  " << config->getDeviceGUIDString();
                    inputOptionList->add(displayName, newInputConfig, false);
                }
            }
        }
        if (configuratedName.compare("") == 0 || !found) {
            LOG(LogWarning) << "adding default entry for player " << player << "(selected : true)";
            inputOptionList->add("default", NULL, true);
        } else {
            LOG(LogWarning) << "adding default entry for player" << player << "(selected : false)";
            inputOptionList->add("default", NULL, false);
        }

        // ADD default config

        // Populate controllers list
        s->addWithLabel(strbuf, inputOptionList);
    }
    s->addSaveFunc([this, options, window] {
        for (int player = 0; player < MAX_PLAYERS; player++) {
            std::stringstream sstm;
            sstm << "INPUT P" << player + 1;
            std::string confName = sstm.str() + "NAME";
            std::string confGuid = sstm.str() + "GUID";

            auto input_p1 = options.at(player);
            std::string name;
            std::string selectedName = input_p1->getSelectedName();

            if (selectedName.compare(strToUpper("default")) == 0) {
	      name = "DEFAULT";
	      Settings::getInstance()->setString(confName, name);
	      Settings::getInstance()->setString(confGuid, "");
            } else {
	      if(input_p1->getSelected() != NULL) {
		LOG(LogWarning) << "Found the selected controller ! : name in list  = " << selectedName;
		LOG(LogWarning) << "Found the selected controller ! : guid  = " << input_p1->getSelected()->deviceGUIDString;

		Settings::getInstance()->setString(confName, input_p1->getSelected()->deviceName);
		Settings::getInstance()->setString(confGuid, input_p1->getSelected()->deviceGUIDString);
	      }
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

    if ((config->isMappedTo("a", input) || config->isMappedTo("start", input)) && input.value != 0) {
        delete this;
        return true;
    }

    return false;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts() {
    std::vector<HelpPrompt> prompts;
    prompts.push_back(HelpPrompt("up/down", _("CHOOSE")));
    prompts.push_back(HelpPrompt("b", _("SELECT")));
    prompts.push_back(HelpPrompt("start", _("CLOSE")));
    return prompts;
}

std::shared_ptr<OptionListComponent<std::string>> GuiMenu::createRatioOptionList(Window *window,
                                                                                 std::string configname) const {
  auto ratio_choice = std::make_shared<OptionListComponent<std::string> >(window, _("GAME RATIO"), false);
    std::string currentRatio = RecalboxConf::getInstance()->get(configname + ".ratio");
    if (currentRatio.empty()) {
        currentRatio = std::string("auto");
    }

    ratio_choice->add(_("CUSTOM"), "custom", currentRatio == "custom");
    ratio_choice->add(_("AUTO"), "auto", currentRatio == "auto");
    ratio_choice->add(_("4/3"), "4/3", currentRatio == "4/3");
    ratio_choice->add(_("16/9"), "16/9", currentRatio == "16/9");

    return ratio_choice;
}

void GuiMenu::clearLoadedInput() {
  for(int i=0; i<mLoadedInput.size(); i++) {
    delete mLoadedInput[i];
  }
  mLoadedInput.clear();
}
