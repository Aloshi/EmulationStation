/* 
 * File:   NetworkThread.cpp
 * Author: matthieu
 * 
 * Created on 6 février 2015, 11:40
 */

#include "NetworkThread.h"
#include "RecalboxSystem.h"
#include "RecalboxConf.h"
#include "guis/GuiMsgBox.h"
#include "Locale.h"

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

	if(RecalboxConf::getInstance()->get("updates.enabled") == "1") {
	  if(RecalboxSystem::getInstance()->canUpdate()){
	    mWindow->displayMessage(_("AN UPDATE IS AVAILABLE FOR YOUR RECALBOX"));
	    mRunning = false;
	  }
	}
    }
}

