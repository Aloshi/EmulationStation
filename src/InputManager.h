#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include <SDL.h>
#include <vector>
#include <map>

class InputConfig;
class Window;

//you should only ever instantiate one of these, by the way
class InputManager
{
public:
	InputManager(Window* window);
	~InputManager();

	void init();
	void deinit();

	void setNumPlayers(int num);
	int getNumPlayers();

	int getNumJoysticks();

	void parseEvent(const SDL_Event& ev);

	InputConfig* getInputConfigByPlayer(int player);

private:
	static const int DEADZONE = 23000;

	Window* mWindow;

	//non-InputManager classes shouldn't use this, as you can easily miss the keyboard
	InputConfig* getInputConfigByDevice(int device);

	int mNumJoysticks;
	int mNumPlayers;
	SDL_Joystick** mJoysticks;
	InputConfig** mInputConfigs;
	InputConfig* mKeyboardInputConfig;
	std::map<int, int>* mPrevAxisValues;
};

#endif
