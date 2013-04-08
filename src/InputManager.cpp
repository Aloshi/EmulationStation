#include "InputManager.h"
#include "InputConfig.h"
#include "Window.h"
#include <iostream>

InputManager::InputManager(Window* window) : mWindow(window)
{
	mJoysticks = NULL;
	mKeyboardInputConfig = NULL;
	mNumJoysticks = 0;
	mNumPlayers = 0;
}

InputManager::~InputManager()
{
	deinit();
}

void InputManager::init()
{
	if(mJoysticks != NULL)
		deinit();

	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	mNumJoysticks = SDL_NumJoysticks();
	mJoysticks = new SDL_Joystick*[mNumJoysticks];
	mInputConfigs = new InputConfig*[mNumJoysticks];
	mPrevAxisValues = new std::map<int, int>[mNumJoysticks];

	for(int i = 0; i < mNumJoysticks; i++)
	{
		mJoysticks[i] = SDL_JoystickOpen(i);
		mInputConfigs[i] = new InputConfig(i);

		for(int k = 0; k < SDL_JoystickNumAxes(mJoysticks[i]); k++)
		{
			mPrevAxisValues[i][k] = 0;
		}
	}

	mKeyboardInputConfig = new InputConfig(DEVICE_KEYBOARD);

	SDL_JoystickEventState(SDL_ENABLE);
}

void InputManager::deinit()
{
	SDL_JoystickEventState(SDL_DISABLE);

	if(!SDL_WasInit(SDL_INIT_JOYSTICK))
		return;

	if(mJoysticks != NULL)
	{
		for(int i = 0; i < mNumJoysticks; i++)
		{
			SDL_JoystickClose(mJoysticks[i]);
			delete mInputConfigs[i];
		}

		delete mKeyboardInputConfig;

		delete[] mJoysticks;
		delete[] mInputConfigs;
		delete[] mPrevAxisValues;
		mJoysticks = NULL;
		mKeyboardInputConfig = NULL;
		mInputConfigs = NULL;
	}

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

int InputManager::getNumJoysticks() { return mNumJoysticks; }

int InputManager::getNumPlayers() { return mNumPlayers; }
void InputManager::setNumPlayers(int num) { mNumPlayers = num; }

InputConfig* InputManager::getInputConfigByDevice(int device)
{
	if(device == DEVICE_KEYBOARD)
		return mKeyboardInputConfig;
	else
		return mInputConfigs[device];
}

InputConfig* InputManager::getInputConfigByPlayer(int player)
{
	if(mKeyboardInputConfig->getPlayerNum() == player)
		return mKeyboardInputConfig;

	for(int i = 0; i < mNumJoysticks; i++)
	{
		if(mInputConfigs[i]->getPlayerNum() == player)
			return mInputConfigs[i];
	}

	std::cout << "Could not find input config for player number " << player << "!\n";
	return NULL;
}

void InputManager::parseEvent(const SDL_Event& ev)
{
	switch(ev.type)
	{
	case SDL_JOYAXISMOTION:
		//if it switched boundaries
		if((abs(ev.jaxis.value) > DEADZONE) != (abs(mPrevAxisValues[ev.jaxis.which][ev.jaxis.axis]) > DEADZONE))
		{
			int normValue;
			if(abs(ev.jaxis.value) <= DEADZONE)
				normValue = 0;
			else
				if(ev.jaxis.value > 0)
					normValue = 1;
				else
					normValue = -1;

			mWindow->input(getInputConfigByDevice(ev.jaxis.which), Input(ev.jaxis.which, TYPE_AXIS, ev.jaxis.axis, normValue, false));
		}

		mPrevAxisValues[ev.jaxis.which][ev.jaxis.axis] = ev.jaxis.value;
		break;

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		mWindow->input(getInputConfigByDevice(ev.jbutton.which), Input(ev.jbutton.which, TYPE_BUTTON, ev.jbutton.button, ev.jbutton.state == SDL_PRESSED, false));
		break;

	case SDL_JOYHATMOTION:
		mWindow->input(getInputConfigByDevice(ev.jhat.which), Input(ev.jhat.which, TYPE_HAT, ev.jhat.hat, ev.jhat.value, false));
		break;

	case SDL_KEYDOWN:
		if(ev.key.keysym.sym == SDLK_F4)
		{
			SDL_Event* quit = new SDL_Event();
			quit->type = SDL_QUIT;
			SDL_PushEvent(quit);
			return;
		}

		mWindow->input(getInputConfigByDevice(DEVICE_KEYBOARD), Input(DEVICE_KEYBOARD, TYPE_KEY, ev.key.keysym.sym, 1, false));
		break;

	case SDL_KEYUP:
		mWindow->input(getInputConfigByDevice(DEVICE_KEYBOARD), Input(DEVICE_KEYBOARD, TYPE_KEY, ev.key.keysym.sym, 0, false));
		break;
	}
}
