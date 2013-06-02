#include "GuiComponent.h"
#include "Window.h"
#include "Log.h"
#include "Renderer.h"

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
	Renderer::translate(mOffset);

	onRender();

	Renderer::translate(-mOffset);
}

void GuiComponent::onRender()
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

//Offset stuff.
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

//Children stuff.
void GuiComponent::addChild(GuiComponent* cmp)
{
	mChildren.push_back(cmp);

	if(cmp->getParent())
		cmp->getParent()->removeChild(cmp);

	cmp->setParent(this);
}

void GuiComponent::removeChild(GuiComponent* cmp)
{
	if(cmp->getParent() != this)
	{
		LOG(LogError) << "Tried to remove child from incorrect parent!";
	}

	cmp->setParent(NULL);

	for(auto i = mChildren.begin(); i != mChildren.end(); i++)
	{
		if(*i == cmp)
		{
			mChildren.erase(i);
			return;
		}
	}
}

void GuiComponent::clearChildren()
{
	mChildren.clear();
}

unsigned int GuiComponent::getChildCount()
{
	return mChildren.size();
}

GuiComponent* GuiComponent::getChild(unsigned int i)
{
	return mChildren.at(i);
}

void GuiComponent::setParent(GuiComponent* parent)
{
	mParent = parent;
}

GuiComponent* GuiComponent::getParent()
{
	return mParent;
}
