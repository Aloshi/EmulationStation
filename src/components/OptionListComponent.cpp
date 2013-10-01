#include "OptionListComponent.h"

OptionListComponent::OptionListComponent(Window* window) : GuiComponent(window),
	mClosedCallback(nullptr)
{
}

void OptionListComponent::setClosedCallback(std::function<void(std::vector<const ListEntry*>)> callback)
{
	mClosedCallback = callback;
}

bool OptionListComponent::input(InputConfig* config, Input input)
{
	return GuiComponent::input(config, input);
}

void OptionListComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	renderChildren(trans);
}

template <typename T>
void OptionListComponent::populate(const std::vector<T>& vec, std::function<ListEntry(const T&)> selector)
{
	for(auto it = vec.begin(); it != vec.end(); it++)
	{
		ListEntry e = selector(*it);
		if(!e.name.empty())
			mEntries.push_back(e);
	}
}

std::vector<const ListEntry*> OptionListComponent::getSelected()
{
	std::vector<const ListEntry*> ret;
	for(auto it = mEntries.begin(); it != mEntries.end(); it++)
	{
		if((*it).selected)
			ret.push_back(&(*it));
	}

	return ret;
}

void OptionListComponent::close()
{
	if(mClosedCallback)
		mClosedCallback(getSelected());

	delete this;
}

