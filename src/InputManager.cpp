#include "InputManager.h"
#include "InputConfig.h"
#include "Window.h"
#include "Log.h"
#include "pugiXML/pugixml.hpp"
#include <boost/filesystem.hpp>
#include "platform.h"

namespace fs = boost::filesystem;

InputManager::InputManager(Window* window) : mWindow(window), 
	mKeyboardInputConfig(NULL),
	mNumJoysticks(0), mNumPlayers(0)
{
}

InputManager::~InputManager()
{
	deinit();
}

void InputManager::init()
{
	if(initialized())
		deinit();

	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	mNumJoysticks = SDL_NumJoysticks();

	for(int i = 0; i < mNumJoysticks; i++)
	{
		SDL_Joystick* joy = SDL_JoystickOpen(i);
		SDL_JoystickID joyId = SDL_JoystickInstanceID(joy);
		mJoysticks.push_back(joy);
		mInputConfigs[joyId] = new InputConfig(i, SDL_JoystickName(joy));
		
		int numAxes = SDL_JoystickNumAxes(joy);
		mPrevAxisValues[joyId] = new int[numAxes];
		std::fill(mPrevAxisValues[joyId], mPrevAxisValues[joyId] + numAxes, 0); //initialize array to 0
	}

	mKeyboardInputConfig = new InputConfig(DEVICE_KEYBOARD, "Keyboard");

	loadConfig();

	SDL_JoystickEventState(SDL_ENABLE);
}

void InputManager::deinit()
{
	SDL_JoystickEventState(SDL_DISABLE);

	if(!initialized())
		return;

	for(auto iter = mJoysticks.begin(); iter != mJoysticks.end(); iter++)
	{
		SDL_JoystickClose(*iter);
	}

	mJoysticks.clear();

	for(auto iter = mInputConfigs.begin(); iter != mInputConfigs.end(); iter++)
	{
		delete iter->second;
	}

	mInputConfigs.clear();

	for(auto iter = mPrevAxisValues.begin(); iter != mPrevAxisValues.end(); iter++)
	{
		delete[] iter->second;
	}

	mPrevAxisValues.clear();

	if(mKeyboardInputConfig != NULL)
	{
		delete mKeyboardInputConfig;
		mKeyboardInputConfig = NULL;
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

InputConfig* InputManager::getInputConfigByDevice(SDL_JoystickID device)
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

	for(auto iter = mInputConfigs.begin(); iter != mInputConfigs.end(); iter++)
	{
		if(iter->second->getPlayerNum() == player)
		{
			return iter->second;
		}
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
		if(ev.key.keysym.sym == SDLK_BACKSPACE && SDL_IsTextInputActive())
		{
			if(mWindow->peekGui() != NULL)
				mWindow->peekGui()->textInput("\b");

			return true;
		}

		if(ev.key.repeat)
			return false;

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

	case SDL_TEXTINPUT:
		if(mWindow->peekGui() != NULL)
			mWindow->peekGui()->textInput(ev.text.text);
		break;

	case SDL_JOYDEVICEADDED:
		deinit();
		init();
		return true;
	}

	return false;
}

void InputManager::loadConfig()
{
	if(!initialized())
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

	bool* configuredDevice = new bool[mNumJoysticks];
	std::fill(configuredDevice, configuredDevice + mNumJoysticks, false);

	for(auto iter = mInputConfigs.begin(); iter != mInputConfigs.end(); iter++)
	{
		iter->second->setPlayerNum(-1);
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
				if(!configuredDevice[i] && SDL_JoystickName(mJoysticks[i]) == devName)
				{
					SDL_JoystickID joyId = SDL_JoystickInstanceID(mJoysticks[i]);
					mInputConfigs[joyId]->loadFromXML(node, mNumPlayers);
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

	delete[] configuredDevice;

	if(mNumPlayers == 0)
	{
		LOG(LogInfo) << "No input configs loaded. Loading default keyboard config.";
		loadDefaultConfig();
	}

	LOG(LogInfo) << "Loaded InputConfig data for " << getNumPlayers() << " devices.";
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

	cfg->mapInput("mastervolup", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_PLUS, 1, true));
	cfg->mapInput("mastervoldown", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_MINUS, 1, true));
}

void InputManager::writeConfig()
{
	if(!initialized())
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
	std::string path = getHomePath();
	path += "/.emulationstation/es_input.cfg";
	return path;
}

bool InputManager::initialized() const
{
	return mKeyboardInputConfig != NULL;
}
