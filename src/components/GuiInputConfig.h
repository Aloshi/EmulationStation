#ifndef _GUIINPUTCONFIG_H_
#define _GUIINPUTCONFIG_H_

#include "../GuiComponent.h"
#include "../InputManager.h"
#include <map>
#include <SDL/SDL.h>

class GuiInputConfig : GuiComponent {
public:
	GuiInputConfig();
	~GuiInputConfig();

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);
private:
	bool mDone;
	int mInputNum;
	int mLastAxis;
	SDL_Joystick* mJoystick;
	static std::string sInputs[];
	static int sInputCount;
	static std::string sConfigPath;

	std::map<int, InputManager::InputButton> mButtonMap;
	std::map<int, InputManager::InputButton> mAxisPosMap;
	std::map<int, InputManager::InputButton> mAxisNegMap;
	void writeConfig();
};

#endif
