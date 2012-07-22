#include "GuiInputConfig.h"
#include "GuiGameList.h"
#include <iostream>
#include <fstream>

std::string GuiInputConfig::sConfigPath = "./input.cfg";
std::string GuiInputConfig::sInputs[] = { "UP", "DOWN", "LEFT", "RIGHT", "BUTTON1", "BUTTON2" };
int GuiInputConfig::sInputCount = 6;

GuiInputConfig::GuiInputConfig()
{
	mInputNum = 0;
	mDone = false;

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);

	if(SDL_NumJoysticks() < 1)
	{
		std::cerr << "Error - GuiInputConfig found no SDL joysticks!\n";
		mJoystick = NULL;
		mDone = true;
		return;
	}else{
		std::cout << "Opening joystick \"" << SDL_JoystickName(0) << "\"\n";
		mJoystick = SDL_JoystickOpen(0);
	}
}

GuiInputConfig::~GuiInputConfig()
{
	Renderer::unregisterComponent(this);
	InputManager::unregisterComponent(this);
}

void GuiInputConfig::onRender()
{
	Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0xFFFFFF);

	if(mDone)
		Renderer::drawCenteredText("All done!", 2, 0x000000);
	else
		Renderer::drawCenteredText("Please press the axis/button for " + sInputs[mInputNum], 2, 0x000000);
}

void GuiInputConfig::onInput(InputManager::InputButton button, bool keyDown)
{
	if(mDone)
	{
		if(keyDown)
		{
			writeConfig(sConfigPath);

			if(mJoystick)
				SDL_JoystickClose(mJoystick);

			InputManager::loadConfig(sConfigPath);
			delete this;
			new GuiGameList();
		}
		return;
	}

	SDL_Event* event = InputManager::lastEvent;
	if(event->type == SDL_JOYBUTTONDOWN)
	{
		mButtonMap[event->jbutton.button] = (InputManager::InputButton)mInputNum;
		std::cout << "	Mapping " << sInputs[mInputNum] << " to button " << event->jbutton.button << "\n";
		mInputNum++;
	}

	if(mInputNum >= sInputCount)
	{
		mDone = true;
		return;
	}
}

void GuiInputConfig::writeConfig(std::string path)
{
	std::ofstream file(path.c_str());

	typedef std::map<int, InputManager::InputButton>::iterator it_type;
	for(it_type iter = mButtonMap.begin(); iter != mButtonMap.end(); iter++)
	{
		file << "BUTTON " << iter->first << " " << iter->second << "\n";
	}

	for(it_type iter = mAxisMap.begin(); iter != mAxisMap.end(); iter++)
	{
		file << "AXIS " << iter->first << " " << iter->second << "\n";
	}

}
