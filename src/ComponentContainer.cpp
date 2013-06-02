#include "ComponentContainer.h"
#include "GuiComponent.h"

void ComponentContainer::addChild(GuiComponent* cmp)
{
	mChildren.push_back(cmp);
}

void ComponentContainer::removeChild(GuiComponent* cmp)
{
	for(auto i = mChildren.begin(); i != mChildren.end(); i++)
	{
		if(*i == cmp)
		{
			mChildren.erase(i);
			return;
		}
	}
}

void ComponentContainer::clearChildren()
{
	mChildren.clear();
}

unsigned int ComponentContainer::getChildCount()
{
	return mChildren.size();
}

GuiComponent* ComponentContainer::getChild(unsigned int i)
{
	return mChildren.at(i);
}
