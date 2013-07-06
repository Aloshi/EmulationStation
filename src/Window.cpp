#include "Window.h"
#include <iostream>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"

Window::Window()
{
	mInputManager = new InputManager(this);
}

Window::~Window()
{
	delete mInputManager;
}

void Window::pushGui(GuiComponent* gui)
{
	mGuiStack.push_back(gui);
}

void Window::removeGui(GuiComponent* gui)
{
	for(auto i = mGuiStack.begin(); i != mGuiStack.end(); i++)
	{
		if(*i == gui)
		{
			mGuiStack.erase(i);
			return;
		}
	}
}

GuiComponent* Window::peekGui()
{
	if(mGuiStack.size() == 0)
		return NULL;

	return mGuiStack.at(mGuiStack.size() - 1);
}

void Window::render()
{
	//there's nothing to render, which should pretty much never happen
	if(mGuiStack.size() == 0)
		std::cout << "guistack empty\n";

	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->render();
	}
}

void Window::init()
{
	mInputManager->init();
	Renderer::init(0, 0);

	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->init();
	}
}

void Window::deinit()
{
	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->deinit();
	}

	mInputManager->deinit();
	Renderer::deinit();
}

void Window::wake()
{
	mInputManager->init();
	Renderer::wake(0, 0);

	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->init();
	}
}

void Window::sleep()
{
	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->deinit();
	}

	mInputManager->deinit();
	Renderer::sleep();
}

void Window::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("mastervolup", input))
	{
		VolumeControl::getInstance()->setVolume(VolumeControl::getInstance()->getVolume() + 5);
	}
	else if(config->isMappedTo("mastervoldown", input))
	{
		VolumeControl::getInstance()->setVolume(VolumeControl::getInstance()->getVolume() - 5);
	}
	else if(peekGui())
		this->peekGui()->input(config, input);
}

void Window::update(int deltaTime)
{
	if(peekGui())
		peekGui()->update(deltaTime);
}

InputManager* Window::getInputManager()
{
	return mInputManager;
}
