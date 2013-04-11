#include "GuiInputConfig.h"
#include "../Window.h"
#include "../Renderer.h"
#include <iostream>
#include "../Font.h"
#include "GuiGameList.h"

static int inputCount = 8;
static std::string inputName[8] = { "Up", "Down", "Left", "Right", "A", "B", "Menu", "Select"};

GuiInputConfig::GuiInputConfig(Window* window, InputConfig* target) : Gui(window), mTargetConfig(target)
{
	mCurInputId = 0;
}

void GuiInputConfig::update(int deltaTime)
{

}

void GuiInputConfig::input(InputConfig* config, Input input)
{
	if(config != mTargetConfig || input.value == 0)
		return;

	if(mCurInputId >= inputCount)
	{
		//done
		if(input.type == TYPE_BUTTON || input.type == TYPE_KEY)
		{
			if(mTargetConfig->getPlayerNum() < mWindow->getInputManager()->getNumPlayers() - 1)
			{
				mWindow->pushGui(new GuiInputConfig(mWindow, mWindow->getInputManager()->getInputConfigByPlayer(mTargetConfig->getPlayerNum() + 1)));
			}else{
				mWindow->getInputManager()->writeConfig();
				GuiGameList::create(mWindow);
			}
			delete this;
		}
	}else{
		if(config->getMappedTo(input).size() > 0)
		{
			mErrorMsg = "Already mapped!";
			return;
		}

		input.configured = true;
		std::cout << "[" << input.string() << "] -> " << inputName[mCurInputId] << "\n";

		config->mapInput(inputName[mCurInputId], input);
		mCurInputId++;
		mErrorMsg = "";
	}
}

void GuiInputConfig::render()
{
	Font* font = Renderer::getDefaultFont(Renderer::MEDIUM);

	std::stringstream stream;
	stream << "PLAYER " << mTargetConfig->getPlayerNum() + 1 << ", press...";
	Renderer::drawText(stream.str(), 10, 10, 0x000000FF, font);

	int y = 14 + font->getHeight();
	for(int i = 0; i < mCurInputId; i++)
	{
		Renderer::drawText(inputName[i], 10, y, 0x00CC00FF, font);
		y += font->getHeight() + 5;
	}

	if(mCurInputId >= inputCount)
	{
		Renderer::drawCenteredText("Basic config done!", 0, (int)(Renderer::getScreenHeight() * 0.6), 0x00CC00FF, font);
		Renderer::drawCenteredText("Press any button to continue.", 0, (int)(Renderer::getScreenHeight() * 0.6) + font->getHeight() + 4, 0x000000FF, font);
	}else{
		Renderer::drawText(inputName[mCurInputId], 10, y, 0x000000FF, font);
	}

	if(!mErrorMsg.empty())
		Renderer::drawCenteredText(mErrorMsg, 0, Renderer::getScreenHeight() - font->getHeight() - 10, 0xFF0000FF, font);
}
