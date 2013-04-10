#include "Window.h"
#include <iostream>
#include "Renderer.h"
#include "AudioManager.h"

Window::Window()
{
	mInputManager = new InputManager(this);
}

Window::~Window()
{
	delete mInputManager;
}

void Window::pushGui(Gui* gui)
{
	mGuiStack.push_back(gui);
}

void Window::removeGui(Gui* gui)
{
	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		if(mGuiStack.at(i) == gui)
		{
			mGuiStack.erase(mGuiStack.begin() + i);
			break;
		}
	}
}

Gui* Window::peekGui()
{
	if(mGuiStack.size() == 0)
		return NULL;

	return mGuiStack.at(mGuiStack.size() - 1);
}

void Window::render()
{
	if(mGuiStack.size() == 0)
		std::cout << "guistack empty\n";

	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->render();
	}
}

void Window::init()
{
	AudioManager::init();
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

	AudioManager::deinit();
	mInputManager->deinit();
	Renderer::deinit();
}

void Window::input(InputConfig* config, Input input)
{
	if(peekGui())
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
