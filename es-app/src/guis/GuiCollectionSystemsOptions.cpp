#include <fstream>

#include "guis/GuiCollectionSystemsOptions.h"

#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiInfoPopup.h"
#include "guis/GuiRandomCollectionOptions.h"
#include "guis/GuiSettings.h"
#include "guis/GuiTextEditPopup.h"
#include "utils/StringUtil.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "Window.h"
#include "SystemData.h"

GuiCollectionSystemsOptions::GuiCollectionSystemsOptions(Window* window) : GuiComponent(window), mMenu(window, "GAME COLLECTION SETTINGS")
{
	initializeMenu();
}

void GuiCollectionSystemsOptions::initializeMenu()
{
	addChild(&mMenu);

	// get collections
	addSystemsToMenu();

	// manage random collection
	addEntry("RANDOM GAME COLLECTION SETTINGS", 0x777777FF, true, [this] { openRandomCollectionSettings(); });


	ComponentListRow row;
	if(CollectionSystemManager::get()->isEditing())
	{
		row.addElement(std::make_shared<TextComponent>(mWindow, "FINISH EDITING '" + Utils::String::toUpper(CollectionSystemManager::get()->getEditingCollection()) + "' COLLECTION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.makeAcceptInputHandler(std::bind(&GuiCollectionSystemsOptions::exitEditMode, this));
		mMenu.addRow(row);
	}
	else
	{
		// add "Create New Custom Collection from Theme"
		std::vector<std::string> unusedFolders = CollectionSystemManager::get()->getUnusedSystemsFromTheme();
		if (unusedFolders.size() > 0)
		{
			addEntry("CREATE NEW CUSTOM COLLECTION FROM THEME", 0x777777FF, true,
			[this, unusedFolders] {
				auto s = new GuiSettings(mWindow, "SELECT THEME FOLDER");
				std::shared_ptr< OptionListComponent<std::string> > folderThemes = std::make_shared< OptionListComponent<std::string> >(mWindow, "SELECT THEME FOLDER", true);

				// add Custom Systems
				for(auto it = unusedFolders.cbegin() ; it != unusedFolders.cend() ; it++ )
				{
					ComponentListRow row;
					std::string name = *it;

					std::function<void()> createCollectionCall = [name, this, s] {
						createCollection(name);
					};
					row.makeAcceptInputHandler(createCollectionCall);

					auto themeFolder = std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(name), Font::get(FONT_SIZE_SMALL), 0x777777FF);
					row.addElement(themeFolder, true);
					s->addRow(row);
				}
				mWindow->pushGui(s);
			});
		}

		row.addElement(std::make_shared<TextComponent>(mWindow, "CREATE NEW CUSTOM COLLECTION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		auto createCustomCollection = [this](const std::string& newVal) {
			std::string name = newVal;
			// we need to store the first Gui and remove it, as it'll be deleted by the actual Gui
			Window* window = mWindow;
			GuiComponent* topGui = window->peekGui();
			window->removeGui(topGui);
			createCollection(name);
		};
		row.makeAcceptInputHandler([this, createCustomCollection] {
			mWindow->pushGui(new GuiTextEditPopup(mWindow, "New Collection Name", "", createCustomCollection, false));
		});
		mMenu.addRow(row);
	}

	bundleCustomCollections = std::make_shared<SwitchComponent>(mWindow);
	bundleCustomCollections->setState(Settings::getInstance()->getBool("UseCustomCollectionsSystem"));
	mMenu.addWithLabel("GROUP UNTHEMED CUSTOM COLLECTIONS", bundleCustomCollections);

	sortAllSystemsSwitch = std::make_shared<SwitchComponent>(mWindow);
	sortAllSystemsSwitch->setState(Settings::getInstance()->getBool("SortAllSystems"));
	mMenu.addWithLabel("SORT CUSTOM COLLECTIONS AND SYSTEMS", sortAllSystemsSwitch);

	toggleSystemNameInCollections = std::make_shared<SwitchComponent>(mWindow);
	toggleSystemNameInCollections->setState(Settings::getInstance()->getBool("CollectionShowSystemInfo"));
	mMenu.addWithLabel("SHOW SYSTEM NAME IN COLLECTIONS", toggleSystemNameInCollections);

	toggleParenthesisInCollections = std::make_shared<SwitchComponent>(mWindow);
	toggleParenthesisInCollections->setState(Settings::getInstance()->getBool("CollectionsShowParenthesisData"));
	mMenu.addWithLabel("SHOW PARENTHESIS IN COLLECTIONS", toggleParenthesisInCollections);

	// double press to remove from favorites
	doublePressToRemoveFavs = std::make_shared<SwitchComponent>(mWindow);
	doublePressToRemoveFavs->setState(Settings::getInstance()->getBool("DoublePressRemovesFromFavs"));
	mMenu.addWithLabel("PRESS (Y) TWICE TO REMOVE FROM FAVS./COLL.", doublePressToRemoveFavs);

	// Add option to select default collection for screensaver
	defaultScreenSaverCollection = std::make_shared< OptionListComponent<std::string> >(mWindow, "ADD/REMOVE GAMES WHILE SCREENSAVER TO", false);
	// Add default option
	std::string defaultScreenSaverCollectionName = Settings::getInstance()->getString("DefaultScreenSaverCollection");
	defaultScreenSaverCollection->add("<DEFAULT>", "", defaultScreenSaverCollectionName == "");

	std::map<std::string, CollectionSystemData> customSystems = CollectionSystemManager::get()->getCustomCollectionSystems();
	// add all enabled Custom Systems
	for(std::map<std::string, CollectionSystemData>::const_iterator it = customSystems.cbegin() ; it != customSystems.cend() ; it++ )
	{
		if (it->second.isEnabled)
			defaultScreenSaverCollection->add(it->second.decl.longName, it->second.decl.name, defaultScreenSaverCollectionName == it->second.decl.name);
	}

	mMenu.addWithLabel("ADD/REMOVE GAMES WHILE SCREENSAVER TO", defaultScreenSaverCollection);

	mMenu.addButton("BACK", "back", std::bind(&GuiCollectionSystemsOptions::applySettings, this));

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiCollectionSystemsOptions::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
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

void GuiCollectionSystemsOptions::createCollection(std::string inName)
{
	CollectionSystemManager* collSysMgr = CollectionSystemManager::get();
	std::string name = collSysMgr->getValidNewCollectionName(inName);

	SystemData* newSys = collSysMgr->addNewCustomCollection(name, true);
	if (!collSysMgr->saveCustomCollection(newSys)) {
		GuiInfoPopup* s = new GuiInfoPopup(mWindow, "Failed creating '" + Utils::String::toUpper(name) + "' Collection. See log for details.", 8000);
		mWindow->setInfoPopup(s);
		return;
	}
	customOptionList->add(name, name, true);
	std::string outAuto = Utils::String::vectorToDelimitedString(autoOptionList->getSelectedObjects(), ",");
	std::string outCustom = Utils::String::vectorToDelimitedString(customOptionList->getSelectedObjects(), ",");
	updateSettings(outAuto, outCustom);
	ViewController::get()->goToSystemView(newSys);

	Window* window = mWindow;
	collSysMgr->setEditMode(name);
	while(window->peekGui() && window->peekGui() != ViewController::get())
		delete window->peekGui();
}

void GuiCollectionSystemsOptions::openRandomCollectionSettings()
{
	mWindow->pushGui(new GuiRandomCollectionOptions(mWindow));
}

void GuiCollectionSystemsOptions::exitEditMode()
{
	CollectionSystemManager::get()->exitEditMode();
	applySettings();
}

GuiCollectionSystemsOptions::~GuiCollectionSystemsOptions()
{

}

void GuiCollectionSystemsOptions::addSystemsToMenu()
{

	std::map<std::string, CollectionSystemData> autoSystems = CollectionSystemManager::get()->getAutoCollectionSystems();

	autoOptionList = std::make_shared< OptionListComponent<std::string> >(mWindow, "SELECT COLLECTIONS", true);

	// add Auto Systems
	for(std::map<std::string, CollectionSystemData>::const_iterator it = autoSystems.cbegin() ; it != autoSystems.cend() ; it++ )
	{
		autoOptionList->add(it->second.decl.longName, it->second.decl.name, it->second.isEnabled);
	}
	mMenu.addWithLabel("AUTOMATIC GAME COLLECTIONS", autoOptionList);

	std::map<std::string, CollectionSystemData> customSystems = CollectionSystemManager::get()->getCustomCollectionSystems();

	customOptionList = std::make_shared< OptionListComponent<std::string> >(mWindow, "SELECT COLLECTIONS", true);

	// add Custom Systems
	for(std::map<std::string, CollectionSystemData>::const_iterator it = customSystems.cbegin() ; it != customSystems.cend() ; it++ )
	{
		customOptionList->add(it->second.decl.longName, it->second.decl.name, it->second.isEnabled);
	}
	mMenu.addWithLabel("CUSTOM GAME COLLECTIONS", customOptionList);
}

void GuiCollectionSystemsOptions::applySettings()
{
	std::string outAuto = Utils::String::vectorToDelimitedString(autoOptionList->getSelectedObjects(), ",");
	std::string prevAuto = Settings::getInstance()->getString("CollectionSystemsAuto");
	std::string outCustom = Utils::String::vectorToDelimitedString(customOptionList->getSelectedObjects(), ",");
	std::string prevCustom = Settings::getInstance()->getString("CollectionSystemsCustom");
	bool outSort = sortAllSystemsSwitch->getState();
	bool prevSort = Settings::getInstance()->getBool("SortAllSystems");
	bool outBundle = bundleCustomCollections->getState();
	bool prevBundle = Settings::getInstance()->getBool("UseCustomCollectionsSystem");
	bool prevShow = Settings::getInstance()->getBool("CollectionShowSystemInfo");
	bool outShow = toggleSystemNameInCollections->getState();
	bool prevParenthesis = Settings::getInstance()->getBool("CollectionsShowParenthesisData");
	bool outParenthesis = toggleParenthesisInCollections->getState();

	// check if custom collection is still enabled for 'collection during screensaver'.
	// if not set 'collection during screensaver' to "" which renders as <DEFAULT>
	std::string enabledCollectionName = "";
	std::vector<std::string> selection = defaultScreenSaverCollection->getSelectedObjects();
	if (selection.size() > 0)
	{
		std::string selectedCollection = selection.at(0);
		if (selectedCollection != "")
		{
			std::vector<std::string> enabledCollections = customOptionList->getSelectedObjects();
			for(auto nameIt = enabledCollections.begin(); nameIt != enabledCollections.end(); nameIt++)
			{
				if(*nameIt == selectedCollection)
				{
					enabledCollectionName = selectedCollection;
					break;
				}
			}
		}
	}

	Settings::getInstance()->setString("DefaultScreenSaverCollection", enabledCollectionName);
	Settings::getInstance()->setBool("DoublePressRemovesFromFavs", doublePressToRemoveFavs->getState());
	Settings::getInstance()->setBool("CollectionsShowParenthesisData", outParenthesis);

	bool needRefreshCollectionSettings = prevAuto != outAuto || prevCustom != outCustom || outSort != prevSort || outBundle != prevBundle
		|| prevShow != outShow || prevParenthesis != outParenthesis;

	if (needRefreshCollectionSettings)
	{
		updateSettings(outAuto, outCustom);
	}
	else
	{
		Settings::getInstance()->saveFile();
	}
	delete this;
}

void GuiCollectionSystemsOptions::updateSettings(std::string newAutoSettings, std::string newCustomSettings)
{
	Settings::getInstance()->setString("CollectionSystemsAuto", newAutoSettings);
	Settings::getInstance()->setString("CollectionSystemsCustom", newCustomSettings);
	Settings::getInstance()->setBool("SortAllSystems", sortAllSystemsSwitch->getState());
	Settings::getInstance()->setBool("UseCustomCollectionsSystem", bundleCustomCollections->getState());
	Settings::getInstance()->setBool("CollectionShowSystemInfo", toggleSystemNameInCollections->getState());
	Settings::getInstance()->setBool("CollectionsShowParenthesisData", toggleParenthesisInCollections->getState());

	Settings::getInstance()->saveFile();

	CollectionSystemManager::get()->loadEnabledListFromSettings();
	CollectionSystemManager::get()->updateSystemsList();

	// Invalidating all collections cache after the changes
	for (auto sysIt = SystemData::sSystemVector.begin(); sysIt != SystemData::sSystemVector.end(); ++sysIt)
	{
    	SystemData* system = *sysIt;

	    if (system->isCollection())
    	{
        	for (auto fileIt = system->getRootFolder()->getChildren().begin(); fileIt != system->getRootFolder()->getChildren().end(); ++fileIt)
        	{
            	CollectionFileData* collectionFile = dynamic_cast<CollectionFileData*>(*fileIt);
            	if (collectionFile != nullptr) collectionFile->refreshMetadata();
        	}
    	}
	}

	ViewController::get()->goToStart();
	ViewController::get()->reloadAll();
}

bool GuiCollectionSystemsOptions::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(config->isMappedTo("b", input) && input.value != 0)
	{
		applySettings();
	}


	return false;
}

std::vector<HelpPrompt> GuiCollectionSystemsOptions::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
