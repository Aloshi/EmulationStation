#ifndef _GUIDETECTDEVICE_H_
#define _GUIDETECTDEVICE_H_

#include "../GuiComponent.h"

class GuiDetectDevice : public GuiComponent
{
public:
	GuiDetectDevice(Window* window);

	bool input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	void done();

	bool mHoldingFinish;
	int mFinishTimer;
	int mCurrentPlayer;
};

#endif
