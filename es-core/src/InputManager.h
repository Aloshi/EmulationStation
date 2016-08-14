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
	InputManager();

	static InputManager* mInstance;

	static const int DEADZONE = 23000;

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
	virtual ~InputManager();

	static InputManager* getInstance();

	void writeDeviceConfig(InputConfig* config);
	void doOnFinish();
	static std::string getConfigPath();
	static std::string getTemporaryConfigPath();

	void init();
	void deinit();

	int getNumJoysticks();
	int getButtonCountByDevice(int deviceId);
	int getNumConfiguredDevices();

	std::string getDeviceGUIDString(int deviceId);

	InputConfig* getInputConfigByDevice(int deviceId);

	bool parseEvent(const SDL_Event& ev, Window* window);
};

#endif
