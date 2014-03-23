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
#include "../Util.h"

#define HOLD_TIME 1000

using namespace Eigen;

GuiDetectDevice::GuiDetectDevice(Window* window, bool firstRun) : GuiComponent(window),
	mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 5))
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

	// title
	mTitle = std::make_shared<TextComponent>(mWindow, firstRun ? "WELCOME" : "CONFIGURE INPUT", 
		Font::get(FONT_SIZE_LARGE), 0x555555FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true, Vector2i(1, 1), GridFlags::BORDER_BOTTOM);

	// device info
	std::stringstream deviceInfo;
	int numDevices = mWindow->getInputManager()->getNumJoysticks();
	
	if(numDevices > 0)
		deviceInfo << numDevices << " GAMEPAD" << (numDevices > 1 ? "S" : "") << " DETECTED";
	else
		deviceInfo << "NO GAMEPADS DETECTED";
	mDeviceInfo = std::make_shared<TextComponent>(mWindow, deviceInfo.str(), Font::get(FONT_SIZE_SMALL), 0x999999FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mDeviceInfo, Vector2i(0, 1), false, true);

	// message
	mMsg1 = std::make_shared<TextComponent>(mWindow, "HOLD A BUTTON ON YOUR DEVICE TO CONFIGURE IT", Font::get(FONT_SIZE_SMALL), 0x777777FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mMsg1, Vector2i(0, 2), false, true);
	mMsg2 = std::make_shared<TextComponent>(mWindow, "PRESS F4 TO QUIT AT ANY TIME", Font::get(FONT_SIZE_SMALL), 0x777777FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mMsg2, Vector2i(0, 3), false, true);

	// currently held device
	mDeviceHeld = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_MEDIUM), 0xFFFFFFFF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mDeviceHeld, Vector2i(0, 4), false, true);

	setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.5f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiDetectDevice::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	// grid
	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mSize.y());
	//mGrid.setRowHeightPerc(1, mDeviceInfo->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(2, mMsg1->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(3, mMsg2->getFont()->getHeight() / mSize.y());
	//mGrid.setRowHeightPerc(4, mDeviceHeld->getFont()->getHeight() / mSize.y());
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
			mDeviceHeld->setText(strToUpper(config->getDeviceName()));
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
		const float t = (float)mHoldTime / HOLD_TIME;
		unsigned int c = (unsigned char)(t * 255);
		mDeviceHeld->setColor((c << 24) | (c << 16) | (c << 8) | 0xFF);
		if(mHoldTime <= 0)
		{
			// picked one!
			mWindow->pushGui(new GuiInputConfig(mWindow, mHoldingConfig, true, mDoneCallback));
			delete this;
		}
	}
}
