#include "guis/GuiCollectionSystemsOptions.h"
#include "guis/GuiMsgBox.h"
#include "Settings.h"
#include "views/ViewController.h"

#include "components/TextComponent.h"
#include "components/OptionListComponent.h"

GuiCollectionSystemsOptions::GuiCollectionSystemsOptions(Window* window) : GuiComponent(window), mMenu(window, "GAME COLLECTION SETTINGS")
{
	initializeMenu();
}

void GuiCollectionSystemsOptions::initializeMenu()
{
	addChild(&mMenu);

	// get virtual systems

	addSystemsToMenu();

	mMenu.addButton("BACK", "back", std::bind(&GuiCollectionSystemsOptions::applySettings, this));

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

GuiCollectionSystemsOptions::~GuiCollectionSystemsOptions()
{
	//mSystemOptions.clear();
}

void GuiCollectionSystemsOptions::addSystemsToMenu()
{

	std::map<std::string, CollectionSystemData> vSystems = CollectionSystemManager::get()->getCollectionSystems();

	autoOptionList = std::make_shared< OptionListComponent<std::string> >(mWindow, "SELECT COLLECTIONS", true);

	// add Systems
	ComponentListRow row;

	for(std::map<std::string, CollectionSystemData>::iterator it = vSystems.begin() ; it != vSystems.end() ; it++ )
	{
		autoOptionList->add(it->second.decl.longName, it->second.decl.name, it->second.isEnabled);
	}
	mMenu.addWithLabel("AUTOMATIC COLLECTIONS", autoOptionList);
}

void GuiCollectionSystemsOptions::applySettings()
{
	std::string out = commaStringToVector(autoOptionList->getSelectedObjects());
	std::string prev = Settings::getInstance()->getString("CollectionSystemsAuto");
	if (out != "" && !CollectionSystemManager::get()->isThemeAutoCompatible())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"Your theme does not support game collections. Please update your theme, or ensure that you use a theme that contains the folders:\n\n• auto-favorites\n• auto-lastplayed\n• auto-allgames\n\nDo you still want to enable collections?",
				"YES", [this, out, prev] {
					if (prev != out)
					{
						updateSettings(out);
					}
					delete this; },
				"NO", [this] { delete this; }));
	}
	else
	{
		if (prev != out)
		{
			updateSettings(out);
		}
		delete this;
	}
}

void GuiCollectionSystemsOptions::updateSettings(std::string newSettings)
{
	Settings::getInstance()->setString("CollectionSystemsAuto", newSettings);
	Settings::getInstance()->saveFile();
	CollectionSystemManager::get()->loadEnabledListFromSettings();
	CollectionSystemManager::get()->updateSystemsList();
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
