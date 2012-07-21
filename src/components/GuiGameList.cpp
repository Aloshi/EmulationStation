#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>

GuiGameList::GuiGameList()
{
	std::cout << "Creating GuiGameList\n";

	mList = new GuiList();
	addChild(mList);

	setSystemId(0);

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);
}

GuiGameList::~GuiGameList()
{
	Renderer::unregisterComponent(this);
	delete mList;

	InputManager::unregisterComponent(this);
}

void GuiGameList::setSystemId(int id)
{
	//make sure the id is within range
	if(id >= (int)SystemData::sSystemVector.size())
		id -= SystemData::sSystemVector.size();
	if(id < 0)
		id += SystemData::sSystemVector.size();

	mSystemId = id;
	mSystem = SystemData::sSystemVector.at(mSystemId);

	updateList();
}

void GuiGameList::onRender()
{
	Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0xFFFFFF);

	Renderer::drawCenteredText(mSystem->getName(), 2, 0x0000FF);
}

void GuiGameList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::BUTTON1 && mSystem->getGameCount() > 0)
	{
		//SDL_EnableKeyRepeat(0, 0);
		if(!keyDown)
			mSystem->launchGame(mList->getSelection());
		//SDL_EnableKeyRepeat(500, 100);
	}

	if(button == InputManager::RIGHT && keyDown)
	{
		setSystemId(mSystemId + 1);
	}
	if(button == InputManager::LEFT && keyDown)
	{
		setSystemId(mSystemId - 1);
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
