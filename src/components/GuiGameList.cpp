#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include <SDL/SDL.h>

GuiGameList::GuiGameList(SystemData* system)
{
	mSystem = system;

	mList = new GuiList();
	updateList();

	addChild(mList);

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);
}

GuiGameList::~GuiGameList()
{
	Renderer::unregisterComponent(this);
	InputManager::unregisterComponent(this);
}

void GuiGameList::onRender()
{
	Renderer::drawRect(0, 0, 640, 480, 0xFFFFFF);

	SDL_Color color = {0, 155, 100};
	Renderer::drawCenteredText(mSystem->getName(), 2, color);
}

void GuiGameList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::BUTTON1 && keyDown)
	{
		SDL_EnableKeyRepeat(0, 0);
		mSystem->launchGame(mList->getSelection());
		SDL_EnableKeyRepeat(500, 100);
	}
}

void GuiGameList::updateList()
{
	mList->clear();

	for(unsigned int i = 0; i < mSystem->getGameCount(); i++)
	{
		GameData* game = mSystem->getGame(i);
		mList->addObject(game->getName(), game);
	}
}
