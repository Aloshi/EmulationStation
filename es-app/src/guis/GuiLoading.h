//
// Created by matthieu on 03/08/15.
//

#ifndef EMULATIONSTATION_ALL_GUILOADING_H
#define EMULATIONSTATION_ALL_GUILOADING_H


#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/BusyComponent.h"


#include <boost/thread.hpp>

class GuiLoading  : public GuiComponent {
public:
    GuiLoading(Window *window, const std::function<void *()> &mFunc, const std::function<void(void *)> &mFunc2);

    virtual ~GuiLoading();

    void render(const Eigen::Affine3f &parentTrans) override;

    bool input(InputConfig *config, Input input) override;

    std::vector<HelpPrompt> getHelpPrompts() override;

    void update(int deltaTime) override;

private:
    BusyComponent mBusyAnim;
    boost::thread *mHandle;
    bool mRunning;
    const std::function<void*()>& mFunc;
    const std::function<void(void *)>& mFunc2;
    void threadLoading();
    void * result;
};


#endif //EMULATIONSTATION_ALL_GUILOADING_H
