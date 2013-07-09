#include "GuiMenu.h"
#include <iostream>
#include <SDL.h>
#include "../Log.h"
#include "../SystemData.h"
#include "GuiGameList.h"
#include "../Settings.h"
#include "GuiSettingsMenu.h"

GuiMenu::GuiMenu(Window* window, GuiGameList* parent) : GuiComponent(window)
{
	mParent = parent;

	std::shared_ptr<Font> font = Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_LARGE);
	mList = new TextListComponent<std::string>(mWindow, 0, font->getHeight() + 2, font);
	mList->setSelectedTextColor(0x0000FFFF);
	populateList();
}

GuiMenu::~GuiMenu()
{
	delete mList;
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	mList->input(config, input);

	if(config->isMappedTo("menu", input) && input.value != 0)
	{
		delete this;
		return true;
	}

	if(config->isMappedTo("a", input) && input.value != 0)
	{
		executeCommand(mList->getSelectedObject());
		return true;
	}

	return false;
}

void GuiMenu::executeCommand(std::string command)
{
	if(command == "exit")
	{
		//push SDL quit event
		SDL_Event* event = new SDL_Event();
		event->type = SDL_QUIT;
		SDL_PushEvent(event);
	}else if(command == "es_reload")
	{
		//reload the game list
		SystemData::loadConfig(SystemData::getConfigPath(), false);
		mParent->setSystemId(0);
	}else if(command == "es_settings")
	{
		mWindow->pushGui(new GuiSettingsMenu(mWindow));
		delete this;
	}else{
		if(system(command.c_str()) != 0)
		{
			LOG(LogWarning) << "(warning: command terminated with nonzero result!)";
		}
	}
}

void GuiMenu::populateList()
{
	mList->clear();

	//if you want to add your own commands to the menu, here is where you need to change!
	//commands added here are called with system() when selected (so are executed as shell commands)
	//the method is GuiList::addObject(std::string displayString, std::string commandString, unsigned int displayHexColor);
	//the list will automatically adjust as items are added to it, this should be the only area you need to change
	//if you want to do something special within ES, override your command in the executeComand() method

	mList->addObject("Settings", "es_settings", 0x0000FFFF);

	mList->addObject("Restart", "sudo shutdown -r now", 0x0000FFFF);
	mList->addObject("Shutdown", "sudo shutdown -h now", 0x0000FFFF);

	mList->addObject("Reload", "es_reload", 0x0000FFFF);

	if(!Settings::getInstance()->getBool("DONTSHOWEXIT"))
		mList->addObject("Exit", "exit", 0xFF0000FF); //a special case; pushes an SDL quit event to the event stack instead of being called by system()
}

void GuiMenu::update(int deltaTime)
{
	mList->update(deltaTime);
}

void GuiMenu::render()
{
	Renderer::drawRect(Renderer::getScreenWidth() / 4, 0, Renderer::getScreenWidth() / 2, Renderer::getScreenHeight(), 0x999999);
	mList->render();
}
