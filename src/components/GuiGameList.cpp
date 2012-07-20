#include "GuiGameList.h"
#include <iostream>

GuiGameList::GuiGameList(SystemData* system)
{
	mSystem = system;

	mList = new GuiList();
	updateList();

	addChild(mList);

	Renderer::registerComponent(this);
}

void GuiGameList::onRender()
{
	Renderer::drawRect(0, 0, 640, 480, 0xFFFFFF);

	SDL_Color color = {0, 155, 100};
	Renderer::drawCenteredText(mSystem->getName(), 2, color);
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
