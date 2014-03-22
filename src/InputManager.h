#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include <SDL.h>
#include <vector>
#include <map>
#include <string>

class InputConfig;
class Window;

//you should only ever instantiate one of these, by the way
class InputManager
{
private:
	static const int DEADZONE = 23000;

	Window* mWindow;

	void loadDefaultKBConfig();

	std::map<SDL_JoystickID, SDL_Joystick*> mJoysticks;
	std::map<SDL_JoystickID, InputConfig*> mInputConfigs;
	InputConfig* mKeyboardInputConfig;

	std::map<SDL_JoystickID, int*> mPrevAxisValues;

	bool initialized() const;

	void addJoystickByDeviceIndex(int id);
	void removeJoystickByJoystickID(SDL_JoystickID id);
	bool loadInputConfig(InputConfig* config); // returns true if successfully loaded, false if not (or didn't exist)

public:
	InputManager(Window* window);
	~InputManager();

	void writeDeviceConfig(InputConfig* config);
	static std::string getConfigPath();

	void init();
	void deinit();

	int getNumJoysticks();
	int getButtonCountByDevice(int deviceId);
	int getNumConfiguredDevices();

	std::string getDeviceGUIDString(int deviceId);

	InputConfig* getInputConfigByDevice(int deviceId);

	bool parseEvent(const SDL_Event& ev);
};

#endif
