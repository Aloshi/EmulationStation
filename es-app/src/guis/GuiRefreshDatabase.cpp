#include "guis/GuiRefreshDatabase.h"
#include "guis/GuiMsgBox.h"
#include "views/ViewController.h"
#include "SystemManager.h"

#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"

GuiRefreshDatabase::GuiRefreshDatabase(Window* window, SystemData* system) : GuiComponent(window),
	mMenu(window, "REFRESH DATA BASE")
{
	addChild(&mMenu);


	//add systems (If we are on a system, do it alone. Otherwise, do all)
	mSystems = std::make_shared< OptionListComponent<SystemData*> >(mWindow, "SCAN THESE SYSTEMS", true);
	for(auto it = SystemManager::getInstance()->getSystems().begin(); it != SystemManager::getInstance()->getSystems().end(); it++)
	{
		if(!(*it)->hasPlatformId(PlatformIds::PLATFORM_IGNORE))
			mSystems->add((*it)->getFullName(), *it, !system || (*it == system));
	}
	mMenu.addWithLabel("Systems", mSystems);

	//Scan for new files to add to the database.
	mAddFiles = std::make_shared<SwitchComponent>(mWindow);
	mAddFiles->setState(true);
	mMenu.addWithLabel("Look for new files", mAddFiles);

	//Mostly useful when Remove Nonexisting is enabled
	mCheckExists = std::make_shared<SwitchComponent>(mWindow);
	mCheckExists->setState(false);
	mMenu.addWithLabel("Check whether files exist", mCheckExists);

	//Potentially deletes data, so default to false.
	mRemoveNonexisting = std::make_shared<SwitchComponent>(mWindow);
	mRemoveNonexisting->setState(false);
	mMenu.addWithLabel("Remove non-existing files", mRemoveNonexisting);

	mMenu.addButton("START", "start", std::bind(&GuiRefreshDatabase::pressedStart, this));
	mMenu.addButton("BACK", "back", [&] { delete this; });

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiRefreshDatabase::pressedStart()
{
	if(!mCheckExists->getState() && mRemoveNonexisting->getState() )
	{
		mWindow->pushGui(new GuiMsgBox(mWindow, 
			strToUpper("Warning: The remove action will not be performed since the check is not being performed.\nContinue anyway?"), 
			"YES", std::bind(&GuiRefreshDatabase::start, this), 
			"NO", nullptr));
		return;
	}

	start();
}

void GuiRefreshDatabase::start()
{
	bool addFiles = mAddFiles->getState();
	bool checkExists = mCheckExists->getState();
	bool removeNonexisting = checkExists && mRemoveNonexisting->getState();
	std::vector<SystemData*> systems = mSystems->getSelectedObjects();

	if((!addFiles && !checkExists && !removeNonexisting) || systems.empty())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"NOTHING TO BE DONE.\nSELECT AT LEAST ONE ACTION AND AT LEAST ONE SYSTEM."));
	}else{
		auto databaseptr = &SystemManager::getInstance()->database();
		int start = databaseptr->totalChanges();
		for(auto sys = systems.begin(); sys != systems.end(); sys++)
		{
			if(addFiles) databaseptr->addMissingFiles(*sys);
			if(checkExists) databaseptr->updateExists(*sys);
			if(removeNonexisting) databaseptr->removeNonexisting(*sys);
			ViewController::get()->getGameListView(*sys).get()->onFilesChanged();
		}
		int updates = databaseptr->totalChanges() - start;
		mWindow->pushGui(new GuiMsgBox(mWindow,
			"NUMBER OF DATABASE CHANGES: " + std::to_string(updates)));
		delete this;
	}
}



bool GuiRefreshDatabase::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;
	
	if(input.value != 0 && config->isMappedTo("b", input))
	{
		delete this;
		return true;
	}

	if((config->isMappedTo("start", input) || config->isMappedTo("select",input)) && input.value != 0)
	{
		// close everything
		Window* window = mWindow;
		while(window->peekGui() && window->peekGui() != ViewController::get())
			delete window->peekGui();
	}


	return false;
}

std::vector<HelpPrompt> GuiRefreshDatabase::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("select", "close"));
	return prompts;
}
