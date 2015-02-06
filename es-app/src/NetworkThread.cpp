/* 
 * File:   NetworkThread.cpp
 * Author: matthieu
 * 
 * Created on 6 février 2015, 11:40
 */

#include "NetworkThread.h"
#include "RetroboxSystem.h"
#include "guis/GuiMsgBox.h"


NetworkThread::NetworkThread(Window* window) : mWindow(window){
    
    // creer le thread
    mFirstRun = true;
    mRunning = true;
    mThreadHandle = new boost::thread(boost::bind(&NetworkThread::run, this));

}

NetworkThread::~NetworkThread() {
    	mThreadHandle->join();
}

void NetworkThread::run(){
    while(mRunning){
        if(mFirstRun){
            boost::this_thread::sleep(boost::posix_time::seconds(15));
            mFirstRun = false;
        }else {
            boost::this_thread::sleep(boost::posix_time::hours(1));
        }

        if(RetroboxSystem::getInstance()->canUpdate()){
            if(RetroboxSystem::getInstance()->canUpdate()){
                mWindow->displayMessage("AN UPDATE IS AVAILABLE FOR YOUR RECALBOX");
                mRunning = false;
            }
        }
    }
}

