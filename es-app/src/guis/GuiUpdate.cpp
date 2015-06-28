#include "guis/GuiUpdate.h"
#include "guis/GuiMsgBox.h"

#include "Window.h"
#include <boost/thread.hpp>
#include <string>
#include "Log.h"
#include "Settings.h"
#include "RecalboxSystem.h"


GuiUpdate::GuiUpdate(Window* window) : GuiComponent(window), mBusyAnim(window)
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
        mLoading = true;
        mPingHandle = new boost::thread(boost::bind(&GuiUpdate::threadPing, this));
        mBusyAnim.setSize(mSize);
}

GuiUpdate::~GuiUpdate()
{
	mPingHandle->join();
}

bool GuiUpdate::input(InputConfig* config, Input input)
{
        return false;
}

std::vector<HelpPrompt> GuiUpdate::getHelpPrompts()
{
	return std::vector<HelpPrompt>();
}

void GuiUpdate::render(const Eigen::Affine3f& parentTrans)
{
        Eigen::Affine3f trans = parentTrans * getTransform();

        renderChildren(trans);

        Renderer::setMatrix(trans);
        Renderer::drawRect(0.f, 0.f, mSize.x(), mSize.y(), 0x00000011);

        if(mLoading)
        mBusyAnim.render(trans);

}

void GuiUpdate::update(int deltaTime) {
        GuiComponent::update(deltaTime);
        mBusyAnim.update(deltaTime);
        
        Window* window = mWindow;
        if(mState == 1){
            window->pushGui(
                           new GuiMsgBox(window, "REALLY UPDATE?", "YES", 
                           [this] { 
                               mState = 2;
                               mLoading = true;
                               mHandle = new boost::thread(boost::bind(&GuiUpdate::threadUpdate, this));

                           }, "NO", [this] { 
                               mState = -1;
                           })
                                   
            );
            mState = 0;
        }
         
        if(mState == 3){
            window->pushGui(
                new GuiMsgBox(window, "NETWORK CONNECTION NEEDED", "OK", 
                [this] {
                    mState = -1;
                })
            );
            mState = 0;
        }
        if(mState == 4){
            window->pushGui(
                new GuiMsgBox(window, "UPDATE OK, THE SYSTEM WILL NOW REBOOT", "OK", 
                [this] {
                    if(runRestartCommand() != 0)
			LOG(LogWarning) << "Reboot terminated with non-zero result!";
                })
            );
            mState = 0;
        }
        if(mState == 5){
            window->pushGui(
                new GuiMsgBox(window, "UPDATE FAILED, THE SYSTEM WILL NOW REBOOT", "OK", 
                [this] {
                    if(runRestartCommand() != 0)
			LOG(LogWarning) << "Reboot terminated with non-zero result!";
                })
            );
            mState = 0;
        }
        if(mState == 6){
            window->pushGui(
                new GuiMsgBox(window, "NO UPDATE AVAILABLE", "OK", 
                [this] {
                    mState = -1;
                }));
            mState = 0;
        }
        if(mState == -1){
            delete this;
        }
}

void GuiUpdate::threadUpdate() 
{
    bool updateOk = RecalboxSystem::getInstance()->updateSystem();
    if(updateOk){
        this->onUpdateOk();
    }else {
        this->onUpdateError();
    }  
}

void GuiUpdate::threadPing() 
{
        if(RecalboxSystem::getInstance()->ping()){
            if(RecalboxSystem::getInstance()->canUpdate()){
                this->onUpdateAvailable();
            }else {
                this->onNoUpdateAvailable();

            }
        }else {
            this->onPingError();
        }
}
void GuiUpdate::onUpdateAvailable() 
{	
    mLoading = false;
    LOG(LogInfo) << "update available" << "\n";	
    mState = 1;
}
void GuiUpdate::onNoUpdateAvailable() 
{	
    mLoading = false;
    LOG(LogInfo) << "no update available" << "\n";	
    mState = 6;
}
void GuiUpdate::onPingError() 
{
    LOG(LogInfo) << "ping nok" << "\n";	
    mLoading = false;
    mState = 3;
}
void GuiUpdate::onUpdateError()
{
    mLoading = false;
    mState = 5;
}

void GuiUpdate::onUpdateOk()
{
    mLoading = false;
    mState = 4;
}