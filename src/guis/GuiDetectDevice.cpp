#include "GuiDetectDevice.h"
#include "../Window.h"
#include "../Renderer.h"
#include "../resources/Font.h"
#include "GuiInputConfig.h"
#include "../components/TextComponent.h"
#include <iostream>
#include <string>
#include <sstream>
#include "../views/ViewController.h"

#define HOLD_TIME 1000

using namespace Eigen;

GuiDetectDevice::GuiDetectDevice(Window* window, bool firstRun) : GuiComponent(window),
	mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 4))
{
	mHoldingConfig = NULL;
	mHoldTime = 0;

	addChild(&mBackground);
	addChild(&mGrid);

	if(firstRun)
	{
		mDoneCallback = [window] {
			window->getViewController()->goToStart();
		};
	}

	mTitle = std::make_shared<TextComponent>(mWindow, firstRun ? "WELCOME TO EMULATIONSTATION" : "SELECT A DEVICE", 
		Font::get(FONT_SIZE_MEDIUM), 0x666666FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);

	std::string msg = (firstRun ? "First, we need to configure your input device!\n" : "");
	msg += "Hold a button on your input device to configure it.\n"
		"Press F4 to quit at any time.";
	mMsg = std::make_shared<TextComponent>(mWindow, msg, Font::get(FONT_SIZE_SMALL), 0x777777FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mMsg, Vector2i(0, 1), false, true);

	std::stringstream deviceInfo;
	int numDevices = mWindow->getInputManager()->getNumJoysticks();
	
	if(numDevices > 0)
		deviceInfo << numDevices << " joystick" << (numDevices > 1 ? "s" : "") << " detected.";
	else
		deviceInfo << "No joysticks detected!";
	mDeviceInfo = std::make_shared<TextComponent>(mWindow, deviceInfo.str(), Font::get(FONT_SIZE_SMALL), 0x888888FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mDeviceInfo, Vector2i(0, 2), false, true);

	mDeviceHeld = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_MEDIUM), 0x777777FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mDeviceHeld, Vector2i(0, 3), false, true);

	setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.5f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiDetectDevice::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	// grid
	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(2, mDeviceInfo->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(3, mDeviceHeld->getFont()->getHeight() / mSize.y());
}

bool GuiDetectDevice::input(InputConfig* config, Input input)
{
	if(input.type == TYPE_BUTTON || input.type == TYPE_KEY)
	{
		if(input.value && mHoldingConfig == NULL)
		{
			// started holding
			mHoldingConfig = config;
			mHoldTime = HOLD_TIME;
			mDeviceHeld->setText(config->getDeviceName());
		}else if(!input.value && mHoldingConfig == config)
		{
			// cancel
			mHoldingConfig = NULL;
			mDeviceHeld->setText("");
		}
	}

	return true;
}

void GuiDetectDevice::update(int deltaTime)
{
	if(mHoldingConfig)
	{
		mHoldTime -= deltaTime;
		if(mHoldTime <= 0)
		{
			// picked one!
			mWindow->pushGui(new GuiInputConfig(mWindow, mHoldingConfig, true, mDoneCallback));
			delete this;
		}
	}
}
