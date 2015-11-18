#include "InputManager.h"
#include "InputConfig.h"
#include "Settings.h"
#include "Window.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include <boost/filesystem.hpp>
#include <utility>
#include "platform.h"
#include "Settings.h"

#define KEYBOARD_GUID_STRING "-1"

// SO HEY POTENTIAL POOR SAP WHO IS TRYING TO MAKE SENSE OF ALL THIS (by which I mean my future self)
// There are like four distinct IDs used for joysticks (crazy, right?)
// 1. Device index - this is the "lowest level" identifier, and is just the Nth joystick plugged in to the system (like /dev/js#).
//    It can change even if the device is the same, and is only used to open joysticks (required to receive SDL events).
// 2. SDL_JoystickID - this is an ID for each joystick that is supposed to remain consistent between plugging and unplugging.
//    ES doesn't care if it does, though.
// 3. "Device ID" - this is something I made up and is what InputConfig's getDeviceID() returns.  
//    This is actually just an SDL_JoystickID (also called instance ID), but -1 means "keyboard" instead of "error."
// 4. Joystick GUID - this is some squashed version of joystick vendor, version, and a bunch of other device-specific things.
//    It should remain the same across runs of the program/system restarts/device reordering and is what I use to identify which joystick to load.

namespace fs = boost::filesystem;

InputManager* InputManager::mInstance = NULL;

InputManager::InputManager() : mKeyboardInputConfig(NULL)
{
}

InputManager::~InputManager()
{
	deinit();
}

InputManager* InputManager::getInstance()
{
	if(!mInstance)
		mInstance = new InputManager();

	return mInstance;
}

void InputManager::init()
{
	if(initialized())
		deinit();

	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, 
		Settings::getInstance()->getBool("BackgroundJoystickInput") ? "1" : "0");
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	SDL_JoystickEventState(SDL_ENABLE);

	// first, open all currently present joysticks
        this->addAllJoysticks();

	mKeyboardInputConfig = new InputConfig(DEVICE_KEYBOARD, -1, "Keyboard", KEYBOARD_GUID_STRING);
	loadInputConfig(mKeyboardInputConfig);
}

void InputManager::addJoystickByDeviceIndex(int id)
{
	assert(id >= 0 && id < SDL_NumJoysticks());
	
	// open joystick & add to our list
	SDL_Joystick* joy = SDL_JoystickOpen(id);
	assert(joy);

	// add it to our list so we can close it again later
	SDL_JoystickID joyId = SDL_JoystickInstanceID(joy);
	mJoysticks[joyId] = joy;

	char guid[65];
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joy), guid, 65);

	// create the InputConfig
	mInputConfigs[joyId] = new InputConfig(joyId, id, SDL_JoystickName(joy), guid);
	if(!loadInputConfig(mInputConfigs[joyId]))
	{
		LOG(LogInfo) << "Added unconfigured joystick " << SDL_JoystickName(joy) << " (GUID: " << guid << ", instance ID: " << joyId << ", device index: " << id << ").";
	}else{
		LOG(LogInfo) << "Added known joystick " << SDL_JoystickName(joy) << " (instance ID: " << joyId << ", device index: " << id << ")";
	}

	// set up the prevAxisValues
	int numAxes = SDL_JoystickNumAxes(joy);
	mPrevAxisValues[joyId] = new int[numAxes];
	std::fill(mPrevAxisValues[joyId], mPrevAxisValues[joyId] + numAxes, 0); //initialize array to 0
}

void InputManager::removeJoystickByJoystickID(SDL_JoystickID joyId)
{
	assert(joyId != -1);

	// delete old prevAxisValues
	auto axisIt = mPrevAxisValues.find(joyId);
	delete[] axisIt->second;
	mPrevAxisValues.erase(axisIt);

	// delete old InputConfig
	auto it = mInputConfigs.find(joyId);
	delete it->second;
	mInputConfigs.erase(it);

	// close the joystick
	auto joyIt = mJoysticks.find(joyId);
	if(joyIt != mJoysticks.end())
	{
		SDL_JoystickClose(joyIt->second);
		mJoysticks.erase(joyIt);
	}else{
		LOG(LogError) << "Could not find joystick to close (instance ID: " << joyId << ")";
	}
        LOG(LogError) << "I removed a joystick";

}

void InputManager::addAllJoysticks()
{
    clearJoystick();
    int numJoysticks = SDL_NumJoysticks();
    for(int i = 0; i < numJoysticks; i++)
    {
            addJoystickByDeviceIndex(i);
    }
}
void InputManager::clearJoystick()
{
    for(auto iter = mJoysticks.begin(); iter != mJoysticks.end(); iter++)
	{
		SDL_JoystickClose(iter->second);
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

}
void InputManager::deinit()
{
	if(!initialized())
		return;

	this->clearJoystick();

        
	if(mKeyboardInputConfig != NULL)
	{
		delete mKeyboardInputConfig;
		mKeyboardInputConfig = NULL;
	}
        
	SDL_JoystickEventState(SDL_DISABLE);
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

int InputManager::getNumJoysticks() { return mJoysticks.size(); }
int InputManager::getButtonCountByDevice(SDL_JoystickID id)
{
	if(id == DEVICE_KEYBOARD)
		return 120; //it's a lot, okay.
	else
		return SDL_JoystickNumButtons(mJoysticks[id]);
}
int InputManager::getAxisCountByDevice(SDL_JoystickID id)
{
	if(id == DEVICE_KEYBOARD)
		return 0; //it's zero, okay.
	else
		return SDL_JoystickNumAxes(mJoysticks[id]);
}
InputConfig* InputManager::getInputConfigByDevice(int device)
{
	if(device == DEVICE_KEYBOARD)
		return mKeyboardInputConfig;
	else
		return mInputConfigs[device];
}

bool InputManager::parseEvent(const SDL_Event& ev, Window* window)
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

			window->input(getInputConfigByDevice(ev.jaxis.which), Input(ev.jaxis.which, TYPE_AXIS, ev.jaxis.axis, normValue, false));
			causedEvent = true;
		}

		mPrevAxisValues[ev.jaxis.which][ev.jaxis.axis] = ev.jaxis.value;
		return causedEvent;

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		window->input(getInputConfigByDevice(ev.jbutton.which), Input(ev.jbutton.which, TYPE_BUTTON, ev.jbutton.button, ev.jbutton.state == SDL_PRESSED, false));
		return true;

	case SDL_JOYHATMOTION:
		window->input(getInputConfigByDevice(ev.jhat.which), Input(ev.jhat.which, TYPE_HAT, ev.jhat.hat, ev.jhat.value, false));
		return true;

	case SDL_KEYDOWN:
		if(ev.key.keysym.sym == SDLK_BACKSPACE && SDL_IsTextInputActive())
		{
			window->textInput("\b");
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

		window->input(getInputConfigByDevice(DEVICE_KEYBOARD), Input(DEVICE_KEYBOARD, TYPE_KEY, ev.key.keysym.sym, 1, false));
		return true;

	case SDL_KEYUP:
		window->input(getInputConfigByDevice(DEVICE_KEYBOARD), Input(DEVICE_KEYBOARD, TYPE_KEY, ev.key.keysym.sym, 0, false));
		return true;

	case SDL_TEXTINPUT:
		window->textInput(ev.text.text);
		break;

	case SDL_JOYDEVICEADDED:
#if defined(__APPLE__)
        addJoystickByDeviceIndex(ev.jdevice.which); // ev.jdevice.which is a device index
#else
        if(! getInputConfigByDevice(ev.jdevice.which)){
            LOG(LogInfo) << "Reinitialize because of SDL_JOYDEVADDED unknown";
            this->init();
        }
#endif
        return true;

	case SDL_JOYDEVICEREMOVED:
#if defined(__APPLE__)
        removeJoystickByJoystickID(ev.jdevice.which); // ev.jdevice.which is an SDL_JoystickID (instance ID)
#else
        LOG(LogInfo) << "Reinitialize because of SDL_JOYDEVICEREMOVED";
        this->init();
#endif
        return false;
	}

	return false;
}

bool InputManager::loadInputConfig(InputConfig* config)
{
	std::string path = getConfigPath();
	if(!fs::exists(path))
		return false;
	
	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());

	if(!res)
	{
		LOG(LogError) << "Error parsing input config: " << res.description();
		return false;
	}

	pugi::xml_node root = doc.child("inputList");
	if(!root)
		return false;

	// looking for a device having the same guid and name, or if not, one with the same guid or in last chance, one with the same name
	pugi::xml_node configNode(NULL);

	bool found_guid = false;
	bool found_exact = false;
	for (pugi::xml_node item = root.child("inputConfig"); item; item = item.next_sibling("inputConfig")) {
	  // check the guid
	  if(strcmp(config->getDeviceGUIDString().c_str(), item.attribute("deviceGUID").value()) == 0) {
	    // found a correct guid
	    found_guid = true; // no more need to check the name only
	    configNode = item;
	    
	    if(strcmp(config->getDeviceName().c_str(), item.attribute("deviceName").value()) == 0) {
	      // found the exact device
	      found_exact = true;
	      configNode = item;
	      break;
	    }
	  }

	  // check for a name if no guid is found
	  if(found_guid == false) {
	    if(strcmp(config->getDeviceName().c_str(), item.attribute("deviceName").value()) == 0) {
	      configNode = item;
	    }
	  }
	}
	    
	if(!configNode)
		return false;

	if(found_exact == false) {
	  LOG(LogInfo) << "Approximative device found using guid=" << configNode.attribute("deviceGUID").value() << " name=" << configNode.attribute("deviceName").value() << ")";
	}
	  
	config->loadFromXML(configNode);
	return true;
}

//used in an "emergency" where no keyboard config could be loaded from the inputmanager config file
//allows the user to select to reconfigure in menus if this happens without having to delete es_input.cfg manually
void InputManager::loadDefaultKBConfig()
{
	InputConfig* cfg = getInputConfigByDevice(DEVICE_KEYBOARD);

	cfg->clear();
	cfg->mapInput("up", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_UP, 1, true));
	cfg->mapInput("down", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_DOWN, 1, true));
	cfg->mapInput("left", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_LEFT, 1, true));
	cfg->mapInput("right", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_RIGHT, 1, true));

	cfg->mapInput("a", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_RETURN, 1, true));
	cfg->mapInput("b", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_ESCAPE, 1, true));
	cfg->mapInput("start", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_F1, 1, true));
	cfg->mapInput("select", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_F2, 1, true));

	cfg->mapInput("pageup", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_RIGHTBRACKET, 1, true));
	cfg->mapInput("pagedown", Input(DEVICE_KEYBOARD, TYPE_KEY, SDLK_LEFTBRACKET, 1, true));
}

void InputManager::writeDeviceConfig(InputConfig* config)
{
	assert(initialized());

	std::string path = getConfigPath();

	pugi::xml_document doc;

	if(fs::exists(path))
	{
		// merge files
		pugi::xml_parse_result result = doc.load_file(path.c_str());
		if(!result)
		{
			LOG(LogError) << "Error parsing input config: " << result.description();
		}else{
			// successfully loaded, delete the old entry if it exists
			pugi::xml_node root = doc.child("inputList");
			if(root)
			{
				pugi::xml_node oldEntry(NULL);
				for (pugi::xml_node item = root.child("inputConfig"); item; item = item.next_sibling("inputConfig")) {
				  if(strcmp(config->getDeviceGUIDString().c_str(), item.attribute("deviceGUID").value()) == 0 &&
				     strcmp(config->getDeviceName().c_str(),       item.attribute("deviceName").value()) == 0) {
				    oldEntry = item;
				    break;
				  }
				}

				if(oldEntry)
					root.remove_child(oldEntry);
				oldEntry = root.find_child_by_attribute("inputConfig", "deviceName", config->getDeviceName().c_str());
				if(oldEntry)
					root.remove_child(oldEntry);
			}
		}
	}

	pugi::xml_node root = doc.child("inputList");
	if(!root)
		root = doc.append_child("inputList");

	config->writeToXML(root);
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

int InputManager::getNumConfiguredDevices()
{
	int num = 0;
	for(auto it = mInputConfigs.begin(); it != mInputConfigs.end(); it++)
	{
		if(it->second->isConfigured())
			num++;
	}

	if(mKeyboardInputConfig->isConfigured())
		num++;

	return num;
}

std::string InputManager::getDeviceGUIDString(int deviceId)
{
	if(deviceId == DEVICE_KEYBOARD)
		return KEYBOARD_GUID_STRING;

	auto it = mJoysticks.find(deviceId);
	if(it == mJoysticks.end())
	{
		LOG(LogError) << "getDeviceGUIDString - deviceId " << deviceId << " not found!";
		return "something went horribly wrong";
	}

	char guid[65];
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(it->second), guid, 65);
	return std::string(guid);
}

std::string InputManager::configureEmulators() {
    std::stringstream command;
    // 1 recuperer les configurated
    
    
    std::list<InputConfig *> availableConfigured;
    

    for (auto it = 0; it < InputManager::getInstance()->getNumJoysticks(); it++) {
        InputConfig * config = InputManager::getInstance()->getInputConfigByDevice(it);
        //LOG(LogInfo) << "I am checking for an input named "<< config->getDeviceName() << " this configured ? "<<config->isConfigured();
        if(config->isConfigured()) {
            availableConfigured.push_back(config);
            LOG(LogInfo) << "Available and configurated : " << config->getDeviceName();
        }
    }
    //2 pour chaque joueur verifier si il y a un configurated
        // associer le input au joueur
        // enlever des disponibles 
    std::map<int, InputConfig*> playerJoysticks;

    for (int player = 0; player < 4; player++) {
        std::stringstream sstm;
        sstm << "INPUT P" << player+1;
        std::string confName = sstm.str();

        std::string playerConfigName = Settings::getInstance()->getString(confName);

        for (std::list<InputConfig *>::iterator it1=availableConfigured.begin(); it1!=availableConfigured.end(); ++it1)
        {
            InputConfig * config = *it1;
            //LOG(LogInfo) << "I am checking for an input named "<< config->getDeviceName() << " this configured ? "<<config->isConfigured();
            //if(!config->isConfigured()) continue;
            bool ifound = playerConfigName.compare(config->getDeviceName()) == 0;
            //LOG(LogInfo) << "I was checking for an input named "<< playerConfigName << " and i compared to "
            //            << config->getDeviceName();
            if(ifound){
                availableConfigured.erase(it1);
                playerJoysticks[player] = config;
                LOG(LogInfo) << "Saved "<< config->getDeviceName() << " for player " << player;
                break;
            }
        }
    }
    
    for (int player = 0; player < 4; player++) {
        InputConfig * playerInputConfig = playerJoysticks[player];
        // si aucune config a été trouvé pour le joueur, on essaie de lui filer un libre
        if(playerInputConfig == NULL){
            LOG(LogInfo) << "No config for player " << player;

            for (std::list<InputConfig *>::iterator it1=availableConfigured.begin(); it1!=availableConfigured.end(); ++it1)
            {
                playerInputConfig = *it1;
                availableConfigured.erase(it1);
                LOG(LogInfo) << "So i set "<< playerInputConfig->getDeviceName() << " for player " << player;
                break;
            }
        }

        if(playerInputConfig != NULL){
            command << "-p" << player+1 << "index " <<  playerInputConfig->getDeviceIndex() << " -p" << player+1 << "guid " << playerInputConfig->getDeviceGUIDString() << " -p" << player+1 << "name \"" <<  playerInputConfig->getDeviceName() << "\" ";
        }/*else {
            command << " " << "DEFAULT" << " -1 DEFAULTDONOTFINDMEINCOMMAND";
        }*/
        
    }
        //LOG(LogInfo) << "I have for "<< "INPUT P" << player << " a configname : " << playerConfigName;
    //command << " \"" << systemName << "\"" ;
    LOG(LogInfo) << "Configure emulators command : " << command.str().c_str();
    return command.str();
}
