#include "GuiList.h"

GuiList::GuiList()
{
	mSelection = 0;
}

void GuiList::onRender()
{
	
}

void GuiList::addObject(std::string name, void* obj)
{
	mNameVector.push_back(name);
	mPointerVector.push_back(obj);
}

void GuiList::clearObjects()
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
