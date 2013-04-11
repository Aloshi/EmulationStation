#include "InputManager.h"
#include "InputConfig.h"
#include "Window.h"
#include <iostream>
#include "Log.h"
#include "pugiXML/pugixml.hpp"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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

	loadConfig();
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

bool InputManager::parseEvent(const SDL_Event& ev)
{
	bool causedEvent = false;
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
			causedEvent = true;
		}

		mPrevAxisValues[ev.jaxis.which][ev.jaxis.axis] = ev.jaxis.value;
		return causedEvent;

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		mWindow->input(getInputConfigByDevice(ev.jbutton.which), Input(ev.jbutton.which, TYPE_BUTTON, ev.jbutton.button, ev.jbutton.state == SDL_PRESSED, false));
		return true;

	case SDL_JOYHATMOTION:
		mWindow->input(getInputConfigByDevice(ev.jhat.which), Input(ev.jhat.which, TYPE_HAT, ev.jhat.hat, ev.jhat.value, false));
		return true;

	case SDL_KEYDOWN:
		if(ev.key.keysym.sym == SDLK_F4)
		{
			SDL_Event* quit = new SDL_Event();
			quit->type = SDL_QUIT;
			SDL_PushEvent(quit);
			return false;
		}

		mWindow->input(getInputConfigByDevice(DEVICE_KEYBOARD), Input(DEVICE_KEYBOARD, TYPE_KEY, ev.key.keysym.sym, 1, false));
		return true;

	case SDL_KEYUP:
		mWindow->input(getInputConfigByDevice(DEVICE_KEYBOARD), Input(DEVICE_KEYBOARD, TYPE_KEY, ev.key.keysym.sym, 0, false));
		return true;
	}

	return false;
}

void InputManager::loadConfig()
{
	if(!mJoysticks)
	{
		std::cout << "ERROR - cannot load config without being initialized!\n";
	}

	std::string path = getConfigPath();
	if(!fs::exists(path))
		return;

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());

	if(!res)
	{
		LOG(LogError) << "Error loading input config: " << res.description();
		return;
	}

	mNumPlayers = 0;

	pugi::xml_node root = doc.child("inputList");

	for(pugi::xml_node node = root.child("inputConfig"); node; node = node.next_sibling("inputConfig"))
	{
		std::string type = node.attribute("type").as_string();

		if(type == "keyboard")
		{
			getInputConfigByDevice(DEVICE_KEYBOARD)->loadFromXML(node, mNumPlayers);
			mNumPlayers++;
		}else if(type == "joystick")
		{
			bool found = false;
			std::string devName = node.child("deviceName").text().get();
			for(int i = 0; i < mNumJoysticks; i++)
			{
				if(SDL_JoystickName(i) == devName)
				{
					mInputConfigs[i]->loadFromXML(node, mNumPlayers);
					mNumPlayers++;
					found = true;
					break;
				}
			}

			if(!found)
			{
				LOG(LogWarning) << "Could not find joystick named \"" << devName << "\"! Skipping it.\n";
				continue;
			}
		}else{
			LOG(LogWarning) << "Device type \"" << type << "\" unknown!\n";
		}
	}
}

void InputManager::writeConfig()
{
	if(!mJoysticks)
	{
		std::cout << "ERROR - cannot write config without being initialized!\n";
		return;
	}

	std::string path = getConfigPath();

	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("inputList");

	mKeyboardInputConfig->writeToXML(root);
	for(int i = 0; i < mNumJoysticks; i++)
	{
		mInputConfigs[i]->writeToXML(root);
	}

	doc.save_file(path.c_str());
}

std::string InputManager::getConfigPath()
{
	std::string path = getenv("HOME");
	path += "/.emulationstation/es_input.cfg";
	return path;
}
