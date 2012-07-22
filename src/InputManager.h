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

	void loadConfig(std::string path);

	//enum for identifying input, regardless of configuration
	enum InputButton { UP, DOWN, LEFT, RIGHT, BUTTON1, BUTTON2, UNKNOWN};

	void processEvent(SDL_Event* event);

	extern std::vector<GuiComponent*> inputVector;
	extern SDL_Event* lastEvent; //mostly for GuiInputConfig

	extern std::map<int, InputButton> joystickButtonMap;
	extern std::map<int, InputButton> joystickAxisMap;
}

#endif
