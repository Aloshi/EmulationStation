#include "InputManager.h"
#include "InputConfig.h"
#include "Window.h"
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
int InputManager::getButtonCountByDevice(int id)
{
	if(id == DEVICE_KEYBOARD)
		return 120; //it's a lot, okay.
	else
		return SDL_JoystickNumButtons(mJoysticks[id]);
}

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

	LOG(LogError) << "Could not find input config for player number " << player << "!";
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
		LOG(LogError) << "ERROR - cannot load InputManager config without being initialized!";
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

	bool configuredDevice[mNumJoysticks];
	for(int i = 0; i < mNumJoysticks; i++)
	{
		mInputConfigs[i]->setPlayerNum(-1);
		configuredDevice[i] = false;
	}

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
			std::string devName = node.attribute("deviceName").as_string();
			for(int i = 0; i < mNumJoysticks; i++)
			{
				if(!configuredDevice[i] && SDL_JoystickName(i) == devName)
				{
					mInputConfigs[i]->loadFromXML(node, mNumPlayers);
					mNumPlayers++;
					found = true;
					configuredDevice[i] = true;
					break;
				}
			}

			if(!found)
			{
				LOG(LogWarning) << "Could not find unconfigured joystick named \"" << devName << "\"! Skipping it.\n";
				continue;
			}
		}else{
			LOG(LogWarning) << "Device type \"" << type << "\" unknown!\n";
		}
	}

	if(mNumPlayers == 0)
	{
		LOG(LogInfo) << "No input configs loaded. Loading default keyboard config.";
		loadDefaultConfig();
	}
}

//used in an "emergency" where no configs could be loaded from the inputmanager config file
//allows the user to select to reconfigure in menus if this happens without having to delete es_input.cfg manually
void InputManager::loadDefaultConfig()
{
	InputConfig* cfg = getInputConfigByDevice(DEVICE_KEYBOARD);

	mNumPlayers++;
	cfg->setPlayerNum(0);
	cfg->mapInput("up", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_UP, 1, true));
	cfg->mapInput("down", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_DOWN, 1, true));
	cfg->mapInput("left", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_LEFT, 1, true));
	cfg->mapInput("right", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_RIGHT, 1, true));

	cfg->mapInput("a", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_RETURN, 1, true));
	cfg->mapInput("b", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_ESCAPE, 1, true));
	cfg->mapInput("menu", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_F1, 1, true));
	cfg->mapInput("select", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_F2, 1, true));
	cfg->mapInput("pageup", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_RIGHTBRACKET, 1, true));
	cfg->mapInput("pagedown", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_LEFTBRACKET, 1, true));
}

void InputManager::writeConfig()
{
	if(!mJoysticks)
	{
		LOG(LogError) << "ERROR - cannot write config without being initialized!";
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
