#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/BusyComponent.h"


#include <thread>

class GuiUpdate : public GuiComponent
{
public:
	GuiUpdate(Window* window);
        virtual ~GuiUpdate();
	void render(const Eigen::Affine3f& parentTrans) override;
	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	void update(int deltaTime) override;

private:
        BusyComponent mBusyAnim;
        bool mLoading;
        int mState;
	std::thread mHandle;
        std::thread mPingHandle;
	void onUpdateError();
	void onUpdateOk();
        void threadSystemCall();
        void pingThread();
        void onPingOk();
        void onPingError();
};
