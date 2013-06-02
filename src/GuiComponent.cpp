#include "GuiComponent.h"
#include "Window.h"

GuiComponent::GuiComponent(Window* window) : mWindow(window), mParent(NULL)
{
}

GuiComponent::~GuiComponent()
{
	mWindow->removeGui(this);

	if(mParent)
		mParent->removeChild(this);
}

bool GuiComponent::input(InputConfig* config, Input input)
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		if(getChild(i)->input(config, input))
			return true;
	}

	return false;
}

void GuiComponent::update(int deltaTime)
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->update(deltaTime);
	}
}

void GuiComponent::render()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->render();
	}
}

void GuiComponent::init()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->init();
	}
}

void GuiComponent::deinit()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->deinit();
	}
}

void GuiComponent::setParent(GuiComponent* parent)
{
	mParent = parent;
}

GuiComponent* GuiComponent::getParent()
{
	return mParent;
}

Vector2i GuiComponent::getGlobalOffset()
{
	if(mParent)
		return mParent->getGlobalOffset() + mOffset;
	else
		return mOffset;
}

Vector2i GuiComponent::getOffset()
{
	return mOffset;
}

void GuiComponent::setOffset(Vector2i offset)
{
	mOffset = offset;
}

void GuiComponent::setOffset(int x, int y)
{
	mOffset.x = x;
	mOffset.y = y;
}
