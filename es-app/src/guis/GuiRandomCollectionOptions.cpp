#include "guis/GuiRandomCollectionOptions.h"

#include "GuiRandomCollectionOptions.h"
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
	addEntry("INCLUDE SYSTEMS", 0x777777FF, true, [this] { selectSystems(); });
	addEntry("INCLUDE AUTO COLLECTIONS", 0x777777FF, true, [this] { selectAutoCollections(); });
	addEntry("INCLUDE CUSTOM COLLECTIONS", 0x777777FF, true, [this] { selectCustomCollections(); });

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
	trimRandom = std::make_shared<NumberList>(mWindow, "MAX GAMES", false);

	// Add default entry
	int maxGames = Settings::getInstance()->getInt("RandomCollectionMaxGames");
	trimRandom->add("ALL", 0, maxGames == 0);

	// add limit values for size of random collection
	for(int i = 5; i <= 50; i = i+5)
	{
		trimRandom->add(std::to_string(i), i, maxGames == i);
	}

	mMenu.addWithLabel("MAX GAMES", trimRandom);

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
	for(auto &sys : SystemData::sSystemVector)
	{
		// we won't iterate all collections
		if (sys->isGameSystem() && !sys->isCollection())
		{
			CollectionSystemDecl sysDecl;
			sysDecl.name = sys->getName();
			sysDecl.longName = sys->getFullName();

			CollectionSystemData newCollectionData;
			newCollectionData.system = sys;
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

void GuiRandomCollectionOptions::selectEntries(std::map<std::string, CollectionSystemData> collection, std::string settingsLabel, int defaultValue, std::vector< SystemGames>* results) {
	auto s = new GuiSettings(mWindow, "INCLUDE GAMES FROM");

	std::map<std::string, int> initValues = Settings::getInstance()->getMap(settingsLabel);

	results->clear();

	for(auto &c : collection)
	{
		CollectionSystemData csd = c.second;
		if (csd.system != CollectionSystemManager::get()->getRandomCollection())
		{
			ComponentListRow row;

			std::string label = csd.decl.longName;
			int selectedValue = defaultValue;

			if (initValues.find(label) != initValues.end())
			{
				int maxForSys = initValues[label];
				// we won't add more than the max and less than 0
				selectedValue = Math::max(Math::min(RANDOM_SYSTEM_MAX, maxForSys), 0);
				mNeedsCollectionRefresh |= selectedValue != maxForSys; // force overwrite of outlier in settings
			}

			initValues[label] = selectedValue;

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
	s->addSaveFunc([this, settingsLabel, initValues, results] { applyGroupSettings(settingsLabel, initValues, results); });
	mWindow->pushGui(s);
}

void GuiRandomCollectionOptions::applyGroupSettings(std::string settingsLabel, const std::map<std::string, int> &initialValues, std::vector<SystemGames> *results)
{
	std::map<std::string, int> currentValues;
	for (auto it = results->begin(); it != results->end(); ++it)
	{
		currentValues[(*it).name] = (*it).gamesSelection->getSelected();
	}
	if (currentValues != initialValues)
	{
		mNeedsCollectionRefresh = true;
		Settings::getInstance()->setMap(settingsLabel, currentValues);
	}
}

void GuiRandomCollectionOptions::saveSettings()
{
	int curTrim = trimRandom->getSelected();
	int prevTrim = Settings::getInstance()->getInt("RandomCollectionMaxGames");
	Settings::getInstance()->setInt("RandomCollectionMaxGames", curTrim);

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
