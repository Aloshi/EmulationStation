#ifndef _GUIINPUTCONFIG_H_
#define _GUIINPUTCONFIG_H_

#include "../Gui.h"
#include <string>

class GuiInputConfig : public Gui
{
public:
	GuiInputConfig(Window* window, InputConfig* target);

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

private:
	std::string mErrorMsg;
	InputConfig* mTargetConfig;
	int mCurInputId;
};

#endif
