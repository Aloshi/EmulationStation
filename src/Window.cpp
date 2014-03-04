#include "Window.h"
#include <iostream>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"
#include "Log.h"
#include "Settings.h"
#include <iomanip>
#include "views/ViewController.h"
#include "components/HelpComponent.h"
#include "components/ImageComponent.h"

Window::Window() : mNormalizeNextUpdate(false), mFrameTimeElapsed(0), mFrameCountElapsed(0), mAverageDeltaTime(10), 
	mAllowSleep(true)
{
	mInputManager = new InputManager(this);
	mViewController = new ViewController(this);
	mHelp = new HelpComponent(this);

	mBackgroundOverlay = new ImageComponent(this);
	mBackgroundOverlay->setImage(":/scroll_gradient.png");

	pushGui(mViewController);
}

Window::~Window()
{
	delete mViewController; // this would get deleted down below, but just to be safe, delete it here
	delete mBackgroundOverlay;

	//delete all our GUIs
	while(peekGui())
		delete peekGui();
	
	delete mHelp;
	delete mInputManager;
}

void Window::pushGui(GuiComponent* gui)
{
	mGuiStack.push_back(gui);
	setHelpPrompts(gui->getHelpPrompts());
}

void Window::removeGui(GuiComponent* gui)
{
	for(auto i = mGuiStack.begin(); i != mGuiStack.end(); i++)
	{
		if(*i == gui)
		{
			i = mGuiStack.erase(i);

			if(i == mGuiStack.end() && mGuiStack.size()) // we just popped the stack and the stack is not empty
				setHelpPrompts(mGuiStack.back()->getHelpPrompts());

			return;
		}
	}
}

GuiComponent* Window::peekGui()
{
	if(mGuiStack.size() == 0)
		return NULL;

	return mGuiStack.back();
}

bool Window::init(unsigned int width, unsigned int height)
{
	if(!Renderer::init(width, height))
	{
		LOG(LogError) << "Renderer failed to initialize!";
		return false;
	}

	mInputManager->init();

	ResourceManager::getInstance()->reloadAll();

	//keep a reference to the default fonts, so they don't keep getting destroyed/recreated
	if(mDefaultFonts.empty())
	{
		mDefaultFonts.push_back(Font::get(FONT_SIZE_SMALL));
		mDefaultFonts.push_back(Font::get(FONT_SIZE_MEDIUM));
		mDefaultFonts.push_back(Font::get(FONT_SIZE_LARGE));
	}

	mBackgroundOverlay->setResize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	// update our help because font sizes probably changed
	if(peekGui())
		setHelpPrompts(peekGui()->getHelpPrompts());

	return true;
}

void Window::deinit()
{
	mInputManager->deinit();
	ResourceManager::getInstance()->unloadAll();
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
	else
	{
		if(peekGui())
			this->peekGui()->input(config, input);
	}
}

void Window::update(int deltaTime)
{
	if(mNormalizeNextUpdate)
	{
		mNormalizeNextUpdate = false;
		if(deltaTime > mAverageDeltaTime)
			deltaTime = mAverageDeltaTime;
	}

	mFrameTimeElapsed += deltaTime;
	mFrameCountElapsed++;
	if(mFrameTimeElapsed > 500)
	{
		mAverageDeltaTime = mFrameTimeElapsed / mFrameCountElapsed;
		
		if(Settings::getInstance()->getBool("DRAWFRAMERATE"))
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(1) << (1000.0f * (float)mFrameCountElapsed / (float)mFrameTimeElapsed) << "fps, ";
			ss << std::fixed << std::setprecision(2) << ((float)mFrameTimeElapsed / (float)mFrameCountElapsed) << "ms";
			mFrameDataString = ss.str();
		}

		mFrameTimeElapsed = 0;
		mFrameCountElapsed = 0;
	}

	if(peekGui())
		peekGui()->update(deltaTime);
}

void Window::render()
{
	Eigen::Affine3f transform = Eigen::Affine3f::Identity();

	const unsigned int drawBGAfter = mGuiStack.size() > 1 ? mGuiStack.size() - 2 : mGuiStack.size();
	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->render(transform);

		if(i == drawBGAfter)
			mBackgroundOverlay->render(transform);
	}

	mHelp->render(transform);

	if(Settings::getInstance()->getBool("DRAWFRAMERATE"))
	{
		Renderer::setMatrix(Eigen::Affine3f::Identity());
		mDefaultFonts.at(1)->drawText(mFrameDataString, Eigen::Vector2f(50, 50), 0xFF00FFFF);
	}
}

void Window::normalizeNextUpdate()
{
	mNormalizeNextUpdate = true;
}

bool Window::getAllowSleep()
{
	return mAllowSleep;
}

void Window::setAllowSleep(bool sleep)
{
	mAllowSleep = sleep;
}

void Window::renderLoadingScreen()
{
	Renderer::setMatrix(Eigen::Affine3f::Identity());
	Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x000000FF);
	mDefaultFonts.at(2)->drawCenteredText("LOADING", 0, (Renderer::getScreenHeight() - mDefaultFonts.at(2)->getHeight()) / 2.0f , 0xFFFFFFFF);
	Renderer::swapBuffers();
}

void Window::setHelpPrompts(const std::vector<HelpPrompt>& prompts)
{
	mHelp->clearPrompts();

	std::map<std::string, bool> seenMap;
	for(auto it = prompts.begin(); it != prompts.end(); it++)
	{
		// only add it if the same icon hasn't already been added
		if(seenMap.insert(std::make_pair<std::string, bool>(it->first, true)).second)
			mHelp->addPrompt(it->first, it->second);
	}
}
