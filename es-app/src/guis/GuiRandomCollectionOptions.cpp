#include "guis/GuiRandomCollectionOptions.h"

#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiSettings.h"
#include "guis/GuiTextEditPopup.h"
#include "utils/StringUtil.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "SystemData.h"
#include "Window.h"

GuiRandomCollectionOptions::GuiRandomCollectionOptions(Window* window) : GuiComponent(window), mMenu(window, "RANDOM COLLECTION")
{
	customCollectionLists.clear();
	autoCollectionLists.clear();
	systemLists.clear();
	mNeedsCollectionRefresh = false;
	
	initializeMenu();
}

void GuiRandomCollectionOptions::initializeMenu()
{
	// get collections
	addEntry("INCLUDED SYSTEMS", 0x777777FF, true, [this] { selectSystems(); });
	addEntry("INCLUDED AUTO COLLECTIONS", 0x777777FF, true, [this] { selectAutoCollections(); });
	addEntry("INCLUDED CUSTOM COLLECTIONS", 0x777777FF, true, [this] { selectCustomCollections(); });
	
	// Add option to exclude games from a collection
	exclusionCollection = std::make_shared< OptionListComponent<std::string> >(mWindow, "EXCLUDE GAMES FROM", false);
	
	// Add default option
	exclusionCollection->add("<NONE>", "", Settings::getInstance()->getString("RandomCollectionExclusionCollection") == "");

	std::map<std::string, CollectionSystemData> customSystems = CollectionSystemManager::get()->getCustomCollectionSystems();
	// add all enabled Custom Systems
	for(std::map<std::string, CollectionSystemData>::const_iterator it = customSystems.cbegin() ; it != customSystems.cend() ; it++ )
	{
		exclusionCollection->add(it->second.decl.longName, it->second.decl.name, Settings::getInstance()->getString("RandomCollectionExclusionCollection") == it->second.decl.name);
	}

	mMenu.addWithLabel("EXCLUDE GAMES FROM", exclusionCollection);
	

	// Add option to trim random collection items
	trimRandom = std::make_shared< OptionListComponent<std::string> >(mWindow, "MAX GAMES", false);
	
	// Add default entry
	trimRandom->add("ALL", "", Settings::getInstance()->getString("RandomCollectionMaxItems") == "");
	
	// add all enabled Custom Systems
	for(int i = 5; i <= 50; i = i+5)
	{
		trimRandom->add(std::to_string(i), std::to_string(i), Settings::getInstance()->getString("RandomCollectionMaxItems") == std::to_string(i));
	}

	mMenu.addWithLabel("MAX ITEMS", trimRandom);

	addChild(&mMenu);

	mMenu.addButton("OK", "ok", std::bind(&GuiRandomCollectionOptions::saveSettings, this));
	mMenu.addButton("CANCEL", "cancel", [&] { delete this; });

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiRandomCollectionOptions::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
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

void GuiRandomCollectionOptions::selectSystems()
{
	std::map<std::string, CollectionSystemData> systems;
	for(auto sysIt = SystemData::sSystemVector.cbegin(); sysIt != SystemData::sSystemVector.cend(); sysIt++)
	{
		// we won't iterate all collections
		if ((*sysIt)->isGameSystem() && !(*sysIt)->isCollection()) 
		{
			CollectionSystemDecl sysDecl;
			sysDecl.name = (*sysIt)->getName();
			sysDecl.longName = (*sysIt)->getFullName();

			CollectionSystemData newCollectionData;
			newCollectionData.system = (*sysIt);
			newCollectionData.decl = sysDecl;
			newCollectionData.isEnabled = true;
			
			systems[sysDecl.name] = newCollectionData;
		}
	}
	selectEntries(systems, "RandomCollectionSystems", DEFAULT_RANDOM_SYSTEM_GAMES, &systemLists);
}

void GuiRandomCollectionOptions::selectAutoCollections()
{
	selectEntries(CollectionSystemManager::get()->getAutoCollectionSystems(), "RandomCollectionSystemsAuto", DEFAULT_RANDOM_COLLECTIONS_GAMES, &autoCollectionLists);
}

void GuiRandomCollectionOptions::selectCustomCollections()
{
	selectEntries(CollectionSystemManager::get()->getCustomCollectionSystems(), "RandomCollectionSystemsCustom", DEFAULT_RANDOM_COLLECTIONS_GAMES, &customCollectionLists);
}

GuiRandomCollectionOptions::~GuiRandomCollectionOptions()
{

}

std::string GuiRandomCollectionOptions::collectionListsToString(std::vector< SystemGames> collectionLists) {
	std::string result;
	for (std::vector< SystemGames>::const_iterator it = collectionLists.cbegin(); it != collectionLists.cend(); it++)
	{
		if (it != collectionLists.cbegin())
			result += ",";

		result += (*it).name + ":" + std::to_string((*it).gamesSelection->getSelected());
	}
	return result;
}

void GuiRandomCollectionOptions::selectEntries(std::map<std::string, CollectionSystemData> collection, std::string settingsLabel, int defaultValue, std::vector< SystemGames>* results) {
	auto s = new GuiSettings(mWindow, "INCLUDE GAMES FROM");
	
	std::map<std::string, int> settingsValues = stringToRandomSettingsMap(Settings::getInstance()->getString(settingsLabel));

	results->clear();

	// add Auto Systems
	for(std::map<std::string, CollectionSystemData>::const_iterator it = collection.cbegin() ; it != collection.cend() ; it++ )
	{
		if (it->second.system != CollectionSystemManager::get()->getRandomCollection())
		{
			ComponentListRow row;

			std::string label = it->second.decl.longName;
			int selectedValue = defaultValue; 

			if (settingsValues.find(label) != settingsValues.end()) 
				selectedValue = Math::min(RANDOM_SYSTEM_MAX, settingsValues[label]);

			std::shared_ptr<NumberList> colItems = std::make_shared<NumberList>(mWindow, label, false);
			for (int i = 0; i <= RANDOM_SYSTEM_MAX; i++)
			{
				colItems->add(std::to_string(i), i, i == selectedValue);
			}
			row.addElement(std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(label), Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			row.addElement(colItems, false);

			s->addRow(row);
			SystemGames sys;
			sys.name = label;
			sys.gamesSelection = colItems;
			results->push_back(sys);
		}

	}
	
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	s->setPosition((mSize.x() - s->getSize().x()) / 2, (mSize.y() - s->getSize().y()) / 2);
	s->addSaveFunc([this, settingsLabel, results] { applyGroupSettings(settingsLabel, results); });
	mWindow->pushGui(s);
}

void GuiRandomCollectionOptions::applyGroupSettings(std::string settingsLabel, std::vector< SystemGames>* results)
{
	std::string curOptions = GuiRandomCollectionOptions::collectionListsToString((*results));
	std::string prevOptions = Settings::getInstance()->getString(settingsLabel);
	
	if (!curOptions.empty() && curOptions != prevOptions) 
	{
		Settings::getInstance()->setString(settingsLabel, curOptions);
		mNeedsCollectionRefresh = true;
	}
}

void GuiRandomCollectionOptions::saveSettings()
{
	std::string curTrim = trimRandom->getSelected();
	std::string prevTrim = Settings::getInstance()->getString("RandomCollectionMaxItems");
	Settings::getInstance()->setString("RandomCollectionMaxItems", curTrim);


	std::string curExclusion = exclusionCollection->getSelected();
	std::string prevExclusion = Settings::getInstance()->getString("RandomCollectionExclusionCollection");
	Settings::getInstance()->setString("RandomCollectionExclusionCollection", curExclusion);

	mNeedsCollectionRefresh |= (curTrim != prevTrim || curExclusion != prevExclusion);

	if (mNeedsCollectionRefresh) 
	{
		Settings::getInstance()->saveFile();
		CollectionSystemManager::get()->recreateCollection(CollectionSystemManager::get()->getRandomCollection());
	}

	delete this;
}

bool GuiRandomCollectionOptions::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(config->isMappedTo("b", input) && input.value != 0)
		saveSettings();

	return false;
}

std::vector<HelpPrompt> GuiRandomCollectionOptions::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
