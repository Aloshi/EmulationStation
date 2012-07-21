#include "GuiList.h"
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
	const int cutoff = 40;
	const int entrySize = 40;

	//number of entries that can fit on the screen simultaniously
	int screenCount = (Renderer::getScreenHeight() - cutoff) / entrySize;
	screenCount -= 1;

	int startEntry = mSelection - (screenCount * 0.5);
	if(startEntry < 0)
		startEntry = 0;
	if(startEntry >= (int)mNameVector.size() - screenCount)
		startEntry = mNameVector.size() - screenCount;


	int y = cutoff;
	int color =  0xFF0000;

	if(mNameVector.size() == 0)
	{
		Renderer::drawCenteredText("The list is empty.", y, color);
		return;
	}

	for(int i = startEntry; i < startEntry + screenCount; i++)
	{
		if(mSelection == i)
		{
			Renderer::drawRect(0, y, Renderer::getScreenWidth(), 52, 0x000000);
		}

		Renderer::drawCenteredText(mNameVector.at((unsigned int)i), y, color);
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
