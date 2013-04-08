#ifndef _GUIINPUTCONFIG_H_
#define _GUIINPUTCONFIG_H_

#include "../Gui.h"
#include <map>
#include <SDL/SDL.h>

class GuiInputConfig : public Gui
{
public:
	GuiInputConfig(Window* window);
	~GuiInputConfig();

	void render();
	void input(InputConfig* config, Input input);
private:
	bool mDone;
	int mInputNum;
	static std::string sInputs[];
	static int sInputCount;
};

#endif
