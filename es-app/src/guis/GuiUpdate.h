#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/BusyComponent.h"


#include <boost/thread.hpp>

class GuiUpdate : public GuiComponent {
public:
    GuiUpdate(Window *window);

    virtual ~GuiUpdate();

    void render(const Eigen::Affine3f &parentTrans) override;

    bool input(InputConfig *config, Input input) override;

    std::vector<HelpPrompt> getHelpPrompts() override;

    void update(int deltaTime) override;

private:
    BusyComponent mBusyAnim;
    bool mLoading;
    int mState;
    boost::thread *mHandle;
    boost::thread *mPingHandle;

    void onUpdateError();

    void onUpdateOk();

    void threadUpdate();

    void threadPing();

    void onUpdateAvailable();

    void onNoUpdateAvailable();

    void onPingError();
};
