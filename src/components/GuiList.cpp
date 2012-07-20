#include "GuiList.h"
#include <SDL/SDL.h>

GuiList::GuiList()
{
	mSelection = 0;
}

void GuiList::onRender()
{
	int y = 40;
	SDL_Color color = {0, 0, 255};
	for(unsigned int i = 0; i < mNameVector.size(); i++)
	{
		Renderer::drawCenteredText(mNameVector.at(i), y, color);
		y += 35;
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
