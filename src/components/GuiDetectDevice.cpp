#include "GuiDetectDevice.h"
#include "../Window.h"
#include "../Renderer.h"
#include "../Font.h"
#include "GuiInputConfig.h"
#include <iostream>
#include <string>
#include <sstream>

GuiDetectDevice::GuiDetectDevice(Window* window) : GuiComponent(window)
{
	//clear any player information from the InputManager
	for(int i = 0; i < mWindow->getInputManager()->getNumPlayers(); i++)
	{
		InputConfig* cfg = mWindow->getInputManager()->getInputConfigByPlayer(i);
		cfg->setPlayerNum(-1);
		cfg->clear();
	}
	mWindow->getInputManager()->setNumPlayers(0);

	mCurrentPlayer = 0;
	mHoldingFinish = false;
}

bool GuiDetectDevice::input(InputConfig* config, Input input)
{
	if((input.type == TYPE_BUTTON || input.type == TYPE_KEY))
	{
		if(config->getPlayerNum() != -1)
		{
			if(config->getPlayerNum() == 0)
			{
				if(input.value)
				{
					mFinishTimer = 0;
					mHoldingFinish = true;
				}else{
					mHoldingFinish = false;
				}
			}
			return true;
		}

		if(!input.value)
			return false;

		//don't allow device list to change once the first player has registered
		if(mCurrentPlayer == 0)
			mWindow->getInputManager()->stopPolling();

		config->setPlayerNum(mCurrentPlayer);
		mWindow->getInputManager()->setNumPlayers(mWindow->getInputManager()->getNumPlayers() + 1); //inc total number of players
		mCurrentPlayer++;

		//mapped everything we possibly can?
		if(mCurrentPlayer >= mWindow->getInputManager()->getNumJoysticks() + 1) //+1 for keyboard
		{
			done();
		}

		return true;
	}

	return false;
}

void GuiDetectDevice::done()
{
	mWindow->pushGui(new GuiInputConfig(mWindow, mWindow->getInputManager()->getInputConfigByPlayer(0)));
	delete this;
}

void GuiDetectDevice::update(int deltaTime)
{
	if(mHoldingFinish)
	{
		mFinishTimer += deltaTime;

		if(mFinishTimer > 1000)
			done();
	}
}

void GuiDetectDevice::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	std::shared_ptr<Font> font = Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM);

	std::string playerString;
	std::stringstream stream;
	stream << (mCurrentPlayer + 1);
	stream >> playerString;

	font->drawCenteredText("Press a button on the device for", 0, Renderer::getScreenHeight() / 3.0f, 0x000000FF);
	font->drawCenteredText("PLAYER " + playerString, 0, (Renderer::getScreenHeight()*1.5f) / 3.0f, 0x333333FF);

	if(mWindow->getInputManager()->getNumPlayers() > 0)
	{
		font->drawCenteredText("(P1 - hold a button to finish)", 0, (Renderer::getScreenHeight()*2) / 3.0f, (mHoldingFinish ? 0x0000FFFF : 0x000066FF));
	}

	if(mWindow->getInputManager()->getNumJoysticks() == 0)
	{
		font->drawCenteredText("No joysticks detected!", 0, Renderer::getScreenHeight() - (font->getHeight()*2.0f)-10, 0xFF0000FF);
	}

	font->drawCenteredText("Press F4 to quit.", 0, Renderer::getScreenHeight() - font->getHeight() - 2.0f , 0x000000FF);
}
