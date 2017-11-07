#pragma once

#include "GuiComponent.h"
#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include "Window.h"
#include "Log.h"



class GuiInfoPopup : public GuiComponent, public Window::InfoPopup
{
public:
	GuiInfoPopup(Window* window, std::string message, int duration);
	~GuiInfoPopup();
	void render(const Eigen::Affine3f& parentTrans) override;
	inline void stop() { running = false; };
private:
	std::string mMessage;
	int mDuration;
	int alpha;
	bool updateState();
	int mStartTime;
	ComponentGrid* mGrid;
	NinePatchComponent* mFrame;
	bool running;
};
