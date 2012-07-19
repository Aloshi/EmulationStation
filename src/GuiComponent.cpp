#include "GuiComponent.h"
#include "Renderer.h"
#include <iostream>

void GuiComponent::addChild(GuiComponent* comp)
{
	mChildren.push_back(comp);
}

void GuiComponent::removeChild(GuiComponent* comp)
{
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		if(mChildren.at(i) == comp)
		{
			mChildren.erase(mChildren.begin() + i);
			break;
		}
	}

	std::cerr << "Error - tried to remove GuiComponent child, but couldn't find it!\n";
}

void GuiComponent::render()
{
	onRender();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->render();
	}
}
