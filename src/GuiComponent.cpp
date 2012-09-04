#include "GuiComponent.h"
#include "Renderer.h"
#include <iostream>

std::vector<GuiComponent*> GuiComponent::sComponentVector;

GuiComponent::GuiComponent()
{
	sComponentVector.push_back(this);
}

GuiComponent::~GuiComponent()
{
	for(unsigned int i = 0; i < sComponentVector.size(); i++)
	{
		if(sComponentVector.at(i) == this)
		{
			sComponentVector.erase(sComponentVector.begin() + i);
		}
	}
}

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

void GuiComponent::clearChildren()
{
	mChildren.clear();
}

void GuiComponent::processTicks(int deltaTime)
{
	for(unsigned int i = 0; i < sComponentVector.size(); i++)
	{
		sComponentVector.at(i)->onTick(deltaTime);
	}
}

void GuiComponent::render()
{
	onRender();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->render();
	}
}

void GuiComponent::pause()
{
	onPause();

	for(unsigned int i = 0; i < mChildren.size(); i++)
		mChildren.at(i)->pause();
}

void GuiComponent::resume()
{
	onResume();

	for(unsigned int i = 0; i < mChildren.size(); i++)
		mChildren.at(i)->resume();
}

void GuiComponent::init()
{
	onInit();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->init();
	}
}

void GuiComponent::deinit()
{
	onDeinit();

	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->deinit();
	}
}
