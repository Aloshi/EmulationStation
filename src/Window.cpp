#include "Window.h"
#include <iostream>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"
#include "Log.h"
#include "Settings.h"
#include <iomanip>

Window::Window() : mNormalizeNextUpdate(false), mFrameTimeElapsed(0), mFrameCountElapsed(0), mAverageDeltaTime(10), 
	mZoomFactor(1.0f), mCenterPoint(0, 0), mMatrix(Eigen::Affine3f::Identity()), mFadePercent(0.0f)
{
	mInputManager = new InputManager(this);
	setCenterPoint(Eigen::Vector2f(Renderer::getScreenWidth() / 2, Renderer::getScreenHeight() / 2));
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
	//there's nothing to render, which should pretty much never happen
	if(mGuiStack.size() == 0)
		std::cout << "guistack empty\n";

	for(unsigned int i = 0; i < mGuiStack.size(); i++)
	{
		mGuiStack.at(i)->render(mMatrix);
	}

	postProcess();

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

InputManager* Window::getInputManager()
{
	return mInputManager;
}

ResourceManager* Window::getResourceManager()
{
	return &mResourceManager;
}

void Window::setZoomFactor(const float& zoom)
{
	mZoomFactor = zoom;
	updateMatrix();
}

void Window::setCenterPoint(const Eigen::Vector2f& point)
{
	mCenterPoint = point;
	updateMatrix();
}

void Window::updateMatrix()
{
	const float sw = Renderer::getScreenWidth() / mZoomFactor;
	const float sh = Renderer::getScreenHeight() / mZoomFactor;

	mMatrix = Eigen::Affine3f::Identity();
	mMatrix = mMatrix.scale(Eigen::Vector3f(mZoomFactor, mZoomFactor, 1));
	mMatrix = mMatrix.translate(Eigen::Vector3f(sw / 2 - mCenterPoint.x(), sh / 2 - mCenterPoint.y(), 0));
}

void Window::setFadePercent(const float& perc)
{
	mFadePercent = perc;
}

void Window::postProcess()
{
	if(mFadePercent > 0.0f)
	{
		Renderer::setMatrix(Eigen::Affine3f::Identity());
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x00000000 | ((unsigned char)(mFadePercent * 255)));
	}
}
