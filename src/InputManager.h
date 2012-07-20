#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include <vector>
#include <SDL/SDL.h>

class GuiComponent;

namespace InputManager {
	void registerComponent(GuiComponent* comp);
	void unregisterComponent(GuiComponent* comp);


	//enum for identifying input type, regardless of configuration
	enum InputButton { UP, DOWN, LEFT, RIGHT, BUTTON1, BUTTON2};


	void processEvent(SDL_Event* event);

	extern std::vector<GuiComponent*> inputVector;
}

#endif
