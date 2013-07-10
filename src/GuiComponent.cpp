#include "GuiComponent.h"
#include "Window.h"
#include "Log.h"
#include "Renderer.h"

GuiComponent::GuiComponent(Window* window) : mWindow(window), mParent(NULL), mOpacity(255), 
	mPosition(Eigen::Vector3f::Zero()), mSize(Eigen::Vector2f::Zero()), mTransform(Eigen::Affine3f::Identity())
{
}

GuiComponent::~GuiComponent()
{
	mWindow->removeGui(this);

	if(mParent)
		mParent->removeChild(this);

	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->setParent(NULL);
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

void GuiComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	renderChildren(trans);
}

void GuiComponent::renderChildren(const Eigen::Affine3f& transform) const
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->render(transform);
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
Eigen::Vector3f GuiComponent::getGlobalPosition()
{
	if(mParent)
		return mParent->getGlobalPosition() + mPosition;
	else
		return mPosition;
}

Eigen::Vector3f GuiComponent::getPosition() const
{
	return mPosition;
}

void GuiComponent::setPosition(const Eigen::Vector3f& offset)
{
	mPosition = offset;
	onPositionChanged();
}

void GuiComponent::setPosition(float x, float y, float z)
{
	mPosition << x, y, z;
	onPositionChanged();
}

Eigen::Vector2f GuiComponent::getSize() const
{
	return mSize;
}

void GuiComponent::setSize(const Eigen::Vector2f& size)
{
    mSize = size;
    onSizeChanged();
}

void GuiComponent::setSize(float w, float h)
{
	mSize << w, h;
    onSizeChanged();
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

unsigned int GuiComponent::getChildCount() const
{
	return mChildren.size();
}

GuiComponent* GuiComponent::getChild(unsigned int i) const
{
	return mChildren.at(i);
}

void GuiComponent::setParent(GuiComponent* parent)
{
	mParent = parent;
}

GuiComponent* GuiComponent::getParent() const
{
	return mParent;
}

unsigned char GuiComponent::getOpacity() const
{
	return mOpacity;
}

void GuiComponent::setOpacity(unsigned char opacity)
{
	mOpacity = opacity;
}

const Eigen::Affine3f GuiComponent::getTransform()
{
	mTransform.setIdentity();
	mTransform.translate(mPosition);
	return mTransform;
}
