#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include <SDL.h>
#include <vector>
#include <map>
#include <string>

class InputConfig;
class Window;

struct InputDevice
{
	std::string name;
	unsigned long vendor;
	unsigned long product;

	InputDevice(const std::string & deviceName, unsigned long vendorId, unsigned long productId);
	bool operator==(const InputDevice & b) const;
};

//you should only ever instantiate one of these, by the way
class InputManager
{
	static const int DEADZONE = 23000;

	Window* mWindow;

	//non-InputManager classes shouldn't use this, as you can easily miss the keyboard
	InputConfig* getInputConfigByDevice(int device);

	void loadDefaultConfig();

	int mNumJoysticks;
	int mNumPlayers;
	SDL_Joystick** mJoysticks;
	InputConfig** mInputConfigs;
	InputConfig* mKeyboardInputConfig;
	std::map<int, int>* mPrevAxisValues;

	std::vector<InputDevice> inputDevices;

	/*!
	Retrieve joysticks/ HID devices from system.
	\return Returns a list of InputDevices that can be compared to the current /inputDevices to check if the configuration has changed.
	\note This currently reads the content of the /dev/input on linux, searches for "js**" names and stores those. On Windows it uses GetRawInputDeviceInfo to find devices of type RIM_TYPEHID and stores those.
	*/
	std::vector<InputDevice> getInputDevices() const;

	static const int POLLING_INTERVAL = 5000;
	SDL_TimerID devicePollingTimer;

	/*!
	Called when devicePollingTimer runs out. Sends a SDL_UserEvent with type SDL_USEREVENT_POLLDEVICES to the event queue.
	*/
	static Uint32 devicePollingCallback(Uint32 interval, void * param);

public:
	static const int SDL_USEREVENT_POLLDEVICES = SDL_USEREVENT + 100; //This event is issued when the input devices should be rescanned.

	InputManager(Window* window);
	~InputManager();

	void loadConfig();
	void writeConfig();
	static std::string getConfigPath();

	void init();
	void deinit();

	void setNumPlayers(int num);
	int getNumPlayers();

	int getNumJoysticks();
	int getButtonCountByDevice(int id);

	bool parseEvent(const SDL_Event& ev);

	InputConfig* getInputConfigByPlayer(int player);

	void startPolling();
	void stopPolling();
};

#endif
