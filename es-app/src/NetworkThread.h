#pragma once

#include "Window.h"
#include <boost/thread.hpp>

class NetworkThread {
public:
    NetworkThread(Window * window);
    virtual ~NetworkThread();
private:
    Window* mWindow;
    bool mRunning;
    bool mFirstRun;
    boost::thread * mThreadHandle;
    void run();
};


