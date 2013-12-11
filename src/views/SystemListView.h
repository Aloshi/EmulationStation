#pragma once

#include "SystemView.h"

class SystemListView : public GuiComponent
{
public:
	SystemListView(Window* window);

	void goToSystem(SystemData* system);
	
	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	Eigen::Affine3f mCamera;

	std::shared_ptr<SystemView> mCurrentView;
	SystemData* mCurrentSystem;
	std::shared_ptr<SystemView> getSystemView(SystemData* system);

	std::map< SystemData*, std::shared_ptr<SystemView> > mSystemViews;
};
