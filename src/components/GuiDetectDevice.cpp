#include "GuiDetectDevice.h"
#include "../Window.h"
#include "../Renderer.h"
#include "../Font.h"
#include "GuiInputConfig.h"
#include <iostream>
#include <string>
#include <sstream>

GuiDetectDevice::GuiDetectDevice(Window* window) : Gui(window)
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

void GuiDetectDevice::input(InputConfig* config, Input input)
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
			return;
		}

		if(!input.value)
			return;

		config->setPlayerNum(mCurrentPlayer);
		mWindow->getInputManager()->setNumPlayers(mWindow->getInputManager()->getNumPlayers() + 1); //inc total number of players
		mCurrentPlayer++;

		//mapped everything we possibly can?
		if(mCurrentPlayer >= mWindow->getInputManager()->getNumJoysticks() + 1) //+1 for keyboard
		{
			done();
		}
	}
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

void GuiDetectDevice::render()
{
	Font* font = Renderer::getDefaultFont(Renderer::MEDIUM);

	std::string playerString;
	std::stringstream stream;
	stream << (mCurrentPlayer + 1);
	stream >> playerString;

	Renderer::drawCenteredText("Press a button on the device for", 0, Renderer::getScreenHeight() / 3, 0x000000FF, font);
	Renderer::drawCenteredText("PLAYER " + playerString, 0, (int)(Renderer::getScreenHeight()*1.5) / 3, 0x333333FF, font);

	if(mWindow->getInputManager()->getNumPlayers() > 0)
	{
		Renderer::drawCenteredText("(P1 - hold a button to finish)", 0, (int)(Renderer::getScreenHeight()*2) / 3, (mHoldingFinish ? 0x0000FFFF : 0x000066FF), font);
	}

	if(mWindow->getInputManager()->getNumJoysticks() == 0)
	{
		Renderer::drawCenteredText("No joysticks detected!", 0, Renderer::getScreenHeight()-(font->getHeight()*2)-10, 0xFF0000FF, font);
	}

	Renderer::drawCenteredText("Press F4 to quit.", 0, Renderer::getScreenHeight()-font->getHeight() - 2 , 0x000000FF, font);
}
