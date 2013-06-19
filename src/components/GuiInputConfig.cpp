#include "GuiInputConfig.h"
#include "../Window.h"
#include "../Renderer.h"
#include "../Font.h"
#include "GuiGameList.h"
#include "../Log.h"

static const int inputCount = 10;
static std::string inputName[inputCount] = { "Up", "Down", "Left", "Right", "A", "B", "Menu", "Select", "PageUp", "PageDown"};
static std::string inputDispName[inputCount] = { "Up", "Down", "Left", "Right", "Accept", "Back", "Menu", "Jump to Letter", "Page Up", "Page Down"};

//MasterVolUp and MasterVolDown are also hooked up, but do not appear on this screen.
//If you want, you can manually add them to es_input.cfg.

GuiInputConfig::GuiInputConfig(Window* window, InputConfig* target) : GuiComponent(window), mTargetConfig(target), mCanSkip(false)
{
	mCurInputId = 0;
	LOG(LogInfo) << "Configuring device " << target->getDeviceId();
}

void GuiInputConfig::update(int deltaTime)
{

}

bool GuiInputConfig::input(InputConfig* config, Input input)
{
	if(config != mTargetConfig || input.value == 0)
		return false;

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
			return true;
		}
	}else{
		if(mCanSkip && config->isMappedTo("a", input))
		{
			mCurInputId++;
			return true;
		}

		if(config->getMappedTo(input).size() > 0)
		{
			mErrorMsg = "Already mapped!";
			return true;
		}

		input.configured = true;
		LOG(LogInfo) << "  [" << input.string() << "] -> " << inputName[mCurInputId];

		config->mapInput(inputName[mCurInputId], input);
		mCurInputId++;
		mErrorMsg = "";

		//for buttons with not enough buttons, press A to skip
		if(mCurInputId >= 7)
		{
			mCanSkip = true;
		}
		return true;
	}

	return false;
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
		Renderer::drawText(inputDispName[i], 10, y, 0x00CC00FF, font);
		y += font->getHeight() + 5;
	}

	if(mCurInputId >= inputCount)
	{
		Renderer::drawCenteredText("Basic config done!", 0, (int)(Renderer::getScreenHeight() * 0.4), 0x00CC00FF, font);
		Renderer::drawCenteredText("Press any button to continue.", 0, (int)(Renderer::getScreenHeight() * 0.4) + font->getHeight() + 4, 0x000000FF, font);
	}else{
		Renderer::drawText(inputDispName[mCurInputId], 10, y, 0x000000FF, font);
		if(mCanSkip)
		{
			int textWidth = 0;
			font->sizeText(inputDispName[mCurInputId], &textWidth, NULL);
			textWidth += 14;

			if(Renderer::getScreenWidth() / 2.5f > textWidth)
				textWidth = (int)(Renderer::getScreenWidth() / 2.5f);

			Renderer::drawText("press Accept to skip", textWidth, y, 0x0000AAFF, font);
		}
	}

	if(!mErrorMsg.empty())
		Renderer::drawCenteredText(mErrorMsg, 0, Renderer::getScreenHeight() - font->getHeight() - 10, 0xFF0000FF, font);
}
