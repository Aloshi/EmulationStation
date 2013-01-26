#include "GuiMenu.h"
#include <iostream>
#include <SDL/SDL.h>
#include "../Log.h"
#include "../SystemData.h"
#include "GuiGameList.h"

//defined in main.cpp
extern bool DONTSHOWEXIT;

GuiMenu::GuiMenu(GuiGameList* parent)
{
	mParent = parent;
	parent->pause();

	mList = new GuiList<std::string>(0, Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, Renderer::getDefaultFont(Renderer::LARGE));
	mList->setSelectedTextColor(0x0000FFFF);
	populateList();
	addChild(mList);

	mSkippedMenuClose = false;

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);
}

GuiMenu::~GuiMenu()
{
	Renderer::unregisterComponent(this);
	InputManager::unregisterComponent(this);

	delete mList;
	mParent->resume();
}

void GuiMenu::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::MENU && !keyDown)
	{
		if(!mSkippedMenuClose)
		{
			mSkippedMenuClose = true;
		}else{
			delete this;
			return;
		}
	}

	if(button == InputManager::BUTTON1 && keyDown)
	{
		executeCommand(mList->getSelectedObject());
	}
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
		SystemData::loadConfig();
		mParent->setSystemId(0);
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
	mList->addObject("Restart", "sudo shutdown -r now", 0x0000FFFF);
	mList->addObject("Shutdown", "sudo shutdown -h now", 0x0000FFFF);

	mList->addObject("Reload", "es_reload", 0x0000FFFF);

	if(!DONTSHOWEXIT)
		mList->addObject("Exit", "exit", 0xFF0000FF); //a special case; pushes an SDL quit event to the event stack instead of being called by system()
}

void GuiMenu::onRender()
{
	Renderer::drawRect(Renderer::getScreenWidth() * 0.25, 0, Renderer::getScreenWidth() * 0.5, Renderer::getScreenHeight(), 0x999999);
}
