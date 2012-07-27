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

	int startEntry = 0;

	//number of entries that can fit on the screen simultaniously
	int screenCount = (Renderer::getScreenHeight() - cutoff) / entrySize;
	screenCount -= 1;

	if((int)mRowVector.size() >= screenCount)
	{
		startEntry = mSelection - (screenCount * 0.5);
		if(startEntry < 0)
			startEntry = 0;
		if(startEntry >= (int)mRowVector.size() - screenCount)
			startEntry = mRowVector.size() - screenCount;
	}

	int y = cutoff;
	int color =  0xFF0000;

	if(mRowVector.size() == 0)
	{
		Renderer::drawCenteredText("The list is empty.", y, color);
		return;
	}

	int listCutoff = startEntry + screenCount;
	if(listCutoff > (int)mRowVector.size())
		listCutoff = mRowVector.size();

	for(int i = startEntry; i < listCutoff; i++)
	{
		if(mSelection == i)
		{
			Renderer::drawRect(0, y, Renderer::getScreenWidth(), 52, 0x000000);
		}

		ListRow row = mRowVector.at((unsigned int)i);
		Renderer::drawCenteredText(row.name, y, row.color);
		y += 40;
	}
}

void GuiList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(mRowVector.size() > 0 && keyDown)
	{
		if(button == InputManager::DOWN)
			mSelection++;

		if(button == InputManager::UP)
			mSelection--;

		if(mSelection < 0)
			mSelection += mRowVector.size();

		if(mSelection >= (int)mRowVector.size())
			mSelection -= mRowVector.size();
	}
}

void GuiList::addObject(std::string name, void* obj, int color)
{
	ListRow row = {name, obj, color};
	mRowVector.push_back(row);
}

void GuiList::clear()
{
	mRowVector.clear();
	mSelection = 0;
}

std::string GuiList::getSelectedName()
{
	return mRowVector.at(mSelection).name;
}

void* GuiList::getSelectedObject()
{
	return mRowVector.at(mSelection).object;
}

int GuiList::getSelection()
{
	return mSelection;
}
