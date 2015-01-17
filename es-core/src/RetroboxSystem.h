/* 
 * File:   RetroboxSystem.h
 * Author: matthieu
 *
 * Created on 29 novembre 2014, 03:15
 */

#ifndef RETROBOXSYSTEM_H
#define	RETROBOXSYSTEM_H
#include <string>


class RetroboxSystem {
public:
    
    
    static RetroboxSystem * getInstance();

    unsigned long getFreeSpaceGB(std::string mountpoint);
    std::string getFreeSpaceInfo();
    bool isFreeSpaceLimit();
    std::string getVersion();
    bool setAudioOutputDevice(std::string device);
    bool setOverscan(bool enable);
    bool setOverclock(std::string mode);

private:
    static RetroboxSystem * instance;
    RetroboxSystem();

};

#endif	/* RETROBOXSYSTEM_Hm */

