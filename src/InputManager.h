#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include <vector>
#include <SDL/SDL.h>
#include <map>
#include <string>

class GuiComponent;

namespace InputManager {
	void registerComponent(GuiComponent* comp);
	void unregisterComponent(GuiComponent* comp);

	void loadConfig();

	//enum for identifying input, regardless of configuration
	enum InputButton { UNKNOWN, UP, DOWN, LEFT, RIGHT, BUTTON1, BUTTON2 };

	void processEvent(SDL_Event* event);

	extern std::vector<GuiComponent*> inputVector;
	extern SDL_Event* lastEvent; //mostly for GuiInputConfig
	extern int deadzone;
	std::string getConfigPath();

	extern std::map<int, InputButton> joystickButtonMap;
	extern std::map<int, InputButton> joystickAxisPosMap, joystickAxisNegMap;
	extern std::map<int, int> axisState;
	extern InputButton hatState;
}

#endif
