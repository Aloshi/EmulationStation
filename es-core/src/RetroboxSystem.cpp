/* 
 * File:   RetroboxSystem.cpp
 * Author: matthieu
 * 
 * Created on 29 novembre 2014, 03:15
 */

#include "RetroboxSystem.h"
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sstream>
#include "Settings.h"
#include <iostream>
#include <fstream>
#include "Log.h"

RetroboxSystem::RetroboxSystem() {
}

RetroboxSystem * RetroboxSystem::instance = NULL;

RetroboxSystem * RetroboxSystem::getInstance() {
    if (RetroboxSystem::instance == NULL) {
        RetroboxSystem::instance = new RetroboxSystem();
    }
    return RetroboxSystem::instance;
}

unsigned long RetroboxSystem::getFreeSpaceGB(std::string mountpoint) {
    struct statvfs fiData;
    const char * fnPath = mountpoint.c_str();
    int free = 0;
    if ((statvfs(fnPath, &fiData)) >= 0) {
        free = (fiData.f_bfree * fiData.f_bsize) / (1024 * 1024 * 1024);
    }
    return free;
}

std::string RetroboxSystem::getFreeSpaceInfo() {
    struct statvfs fiData;
    std::string sharePart = Settings::getInstance()->getString("SharePartition");
    if(sharePart.size() > 0){
        const char * fnPath = sharePart.c_str();
        if ((statvfs(fnPath, &fiData)) < 0) {
            return "";
        } else {
            unsigned long total = (fiData.f_blocks * (fiData.f_bsize / 1024)) / (1024L * 1024L);
            unsigned long free = (fiData.f_bfree * (fiData.f_bsize / 1024)) / (1024L * 1024L);
            unsigned long used = total - free;
            unsigned long percent = used * 100 / total;

            std::ostringstream oss;
            oss << used << "GB/" << total << "GB (" << percent << "%)";
            return oss.str();
        }
    } else {
        return "ERROR";
    }
}

bool RetroboxSystem::isFreeSpaceLimit() {
    std::string sharePart = Settings::getInstance()->getString("SharePartition");
    if(sharePart.size() > 0){
        return getFreeSpaceGB(sharePart) < 2;
    } else {
        return "ERROR";
    }
    
}

std::string RetroboxSystem::getVersion() {
    std::string version = Settings::getInstance()->getString("VersionFile");
    if (version.size() > 0) {
        std::ifstream ifs(version);

        if (ifs.good()) {
            std::string contents;
            std::getline(ifs,contents);
            return contents;
        }
    }
    return "";
}

bool RetroboxSystem::setAudioOutputDevice(std::string device) {
    int commandValue = -1;
    int returnValue = false;
    
    if(device == "auto"){
        commandValue = 0;
    }else if(device == "jack"){
        commandValue = 1;
    }else if(device == "hdmi"){            
        commandValue = 2;
    }else {
        LOG(LogWarning) << "Unable to find audio output device to use !";
    }
    
    if(commandValue != -1){
        std::ostringstream oss;
        oss << "amixer cset numid=3 " << commandValue;
        std::string command = oss.str();
        LOG(LogInfo) << "Launching " << command;
        if(system(command.c_str())){
            LOG(LogWarning) << "Error executing " << command;
            returnValue = false;
        }else {
            LOG(LogInfo) << "Audio output device set to : " << device;
            returnValue = true;
        }
      }
    return returnValue;
}



bool RetroboxSystem::setOverscan(bool enable){
   
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RetroboxSettingScript") << " " << "overscan";
    if(enable){
        oss << " " << "enable";
    }else {
        oss << " " << "disable";
    }
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if(system(command.c_str())){
        LOG(LogWarning) << "Error executing " << command;
        return false;
    }else {
        LOG(LogInfo) << "Overscan set to : " << enable;
        return true;
    }
      
}
bool RetroboxSystem::setOverclock(std::string mode){
    if(mode != ""){
        std::ostringstream oss;
        oss << Settings::getInstance()->getString("RetroboxSettingScript") << " " 
                << "overclock" << " " << mode;
        std::string command = oss.str();
        LOG(LogInfo) << "Launching " << command;
        if(system(command.c_str())){
            LOG(LogWarning) << "Error executing " << command;
            return false;
        }else {
            LOG(LogInfo) << "Overclocking set to " << mode;
            return true;
        }
      }
}