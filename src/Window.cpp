#include "Window.h"
#include <iostream>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"
#include "Log.h"
#include "Settings.h"

Window::Window()
{
	mInputManager = new InputManager(this);
}

Window::~Window()
{
	//delete all our GUIs
	while(peekGui())
		delete peekGui();
	
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

	Eigen::Affine3f trans(Eigen::Affine3f::Identity());

	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->render(trans);
	}
}

bool Window::init(unsigned int width, unsigned int height)
{
	if(!Renderer::init(width, height))
	{
		LOG(LogError) << "Renderer failed to initialize!";
		return false;
	}

	mInputManager->init();

	mResourceManager.reloadAll();

	//keep a reference to the default fonts, so they don't keep getting destroyed/recreated
	if(mDefaultFonts.empty())
	{
		mDefaultFonts.push_back(Font::get(mResourceManager, Font::getDefaultPath(), FONT_SIZE_SMALL));
		mDefaultFonts.push_back(Font::get(mResourceManager, Font::getDefaultPath(), FONT_SIZE_MEDIUM));
		mDefaultFonts.push_back(Font::get(mResourceManager, Font::getDefaultPath(), FONT_SIZE_LARGE));
	}

	return true;
}

void Window::deinit()
{
	mInputManager->deinit();
	mResourceManager.unloadAll();
	Renderer::deinit();
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

ResourceManager* Window::getResourceManager()
{
	return &mResourceManager;
}
