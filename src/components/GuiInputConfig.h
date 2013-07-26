#ifndef _GUIINPUTCONFIG_H_
#define _GUIINPUTCONFIG_H_

#include "../GuiComponent.h"
#include <string>

class GuiInputConfig : public GuiComponent
{
public:
	GuiInputConfig(Window* window, InputConfig* target);

	bool input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	std::string mErrorMsg;
	InputConfig* mTargetConfig;
	int mCurInputId;
	bool mCanSkip;
};

#endif
