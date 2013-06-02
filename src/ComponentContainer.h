#ifndef _COMPONENTCONTAINER_H_
#define _COMPONENTCONTAINER_H_

#include <vector>

class GuiComponent;

/**
	Generic container for Components.
**/

class ComponentContainer
{
public:
	void addChild(GuiComponent* cmp);
	void removeChild(GuiComponent* cmp);
	void clearChildren();
	unsigned int getChildCount();
	GuiComponent* getChild(unsigned int i);

private:
	std::vector<GuiComponent*> mChildren;
};

#endif