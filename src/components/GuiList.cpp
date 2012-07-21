#include "GuiList.h"
#include <SDL/SDL.h>
#include <iostream>

GuiList::GuiList()
{
	mSelection = 0;
	InputManager::registerComponent(this);
}

GuiList::~GuiList()
{
	InputManager::unregisterComponent(this);
}

void GuiList::onRender()
{
	int y = 40;
	SDL_Color color = {0, 0, 255};

	if(mNameVector.size() == 0)
	{
		Renderer::drawCenteredText("The list is empty.", y, color);
		return;
	}

	for(unsigned int i = 0; i < mNameVector.size(); i++)
	{
		if(mSelection == (int)i)
		{
			Renderer::drawRect(0, y, Renderer::getScreenWidth(), 52, 0x000000);
		}

		Renderer::drawCenteredText(mNameVector.at(i), y, color);
		y += 40;
	}
}

void GuiList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(mNameVector.size() > 0 && keyDown)
	{
		if(button == InputManager::DOWN)
			mSelection++;

		if(button == InputManager::UP)
			mSelection--;

		if(mSelection < 0)
			mSelection += mNameVector.size();

		if(mSelection >= (int)mNameVector.size())
			mSelection -= mNameVector.size();
	}
}

void GuiList::addObject(std::string name, void* obj)
{
	mNameVector.push_back(name);
	mPointerVector.push_back(obj);
}

void GuiList::clear()
{
	mNameVector.clear();
	mPointerVector.clear();
}

std::string GuiList::getSelectedName()
{
	return mNameVector.at(mSelection);
}

void* GuiList::getSelectedObject()
{
	return mPointerVector.at(mSelection);
}

int GuiList::getSelection()
{
	return mSelection;
}
