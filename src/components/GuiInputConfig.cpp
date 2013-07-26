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
				mWindow->getInputManager()->startPolling(); //enable polling again since we're done
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

void GuiInputConfig::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	std::shared_ptr<Font> font = Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM);

	std::stringstream stream;
	stream << "PLAYER " << mTargetConfig->getPlayerNum() + 1 << ", press...";
	font->drawText(stream.str(), Eigen::Vector2f(10, 10), 0x000000FF);

	int y = 14 + font->getHeight();
	for(int i = 0; i < mCurInputId; i++)
	{
		font->drawText(inputDispName[i], Eigen::Vector2f(10, y), 0x00CC00FF);
		y += font->getHeight() + 5;
	}

	if(mCurInputId >= inputCount)
	{
		font->drawCenteredText("Basic config done!", 0, Renderer::getScreenHeight() * 0.4f, 0x00CC00FF);
		font->drawCenteredText("Press any button to continue.", 0, Renderer::getScreenHeight() * 0.4f + font->getHeight() + 4, 0x000000FF);
	}else{
		font->drawText(inputDispName[mCurInputId], Eigen::Vector2f(10, y), 0x000000FF);
		if(mCanSkip)
		{
			Eigen::Vector2f textSize = font->sizeText(inputDispName[mCurInputId]);
			textSize[0] += 14;

			if(Renderer::getScreenWidth() / 2.5f > textSize.x())
				textSize[0] = Renderer::getScreenWidth() / 2.5f;

			font->drawText("press Accept to skip", Eigen::Vector2f(textSize.x(), y), 0x0000AAFF);
		}
	}

	if(!mErrorMsg.empty())
		font->drawCenteredText(mErrorMsg, 0, (float)Renderer::getScreenHeight() - font->getHeight() - 10, 0xFF0000FF);
}
