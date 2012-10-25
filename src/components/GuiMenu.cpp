#include "GuiMenu.h"
#include <iostream>

GuiMenu::GuiMenu(GuiComponent* parent)
{
	mParent = parent;
	parent->pause();

	mList = new GuiList<std::string>(0, Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, Renderer::LARGE);
	mList->setSelectedTextColor(0x0000FFFF);
	populateList();
	addChild(mList);

	mSkippedMenuClose = false;

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);
}

GuiMenu::~GuiMenu()
{
	Renderer::unregisterComponent(this);
	InputManager::unregisterComponent(this);

	delete mList;
	mParent->resume();
}

void GuiMenu::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::MENU && !keyDown)
	{
		if(!mSkippedMenuClose)
		{
			mSkippedMenuClose = true;
		}else{
			delete this;
			return;
		}
	}

	if(button == InputManager::BUTTON1 && keyDown)
	{
		system(mList->getSelectedObject().c_str());
	}
}

void GuiMenu::populateList()
{
	mList->clear();

	mList->addObject("Restart", "sudo shutdown -r now", 0x0000FFFF);
	mList->addObject("Shutdown", "sudo shutdown -h now", 0x0000FFFF);
}

void GuiMenu::onRender()
{
	Renderer::drawRect(Renderer::getScreenWidth() * 0.25, 0, Renderer::getScreenWidth() * 0.5, Renderer::getScreenHeight(), 0x999999);
}
