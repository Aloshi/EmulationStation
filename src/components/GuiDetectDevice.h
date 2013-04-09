#ifndef _GUIDETECTDEVICE_H_
#define _GUIDETECTDEVICE_H_

#include "../Gui.h"

class GuiDetectDevice : public Gui
{
public:
	GuiDetectDevice(Window* window);

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

private:
	void done();

	bool mHoldingFinish;
	int mFinishTimer;
	int mCurrentPlayer;
};

#endif
