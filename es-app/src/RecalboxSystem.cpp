/* 
 * File:   RetroboxSystem.cpp
 * Author: matthieu
 * 
 * Created on 29 novembre 2014, 03:15
 */

#include "RecalboxSystem.h"
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sstream>
#include "Settings.h"
#include <iostream>
#include <fstream>
#include "Log.h"
#include "HttpReq.h"

#include "AudioManager.h"
#include "VolumeControl.h"

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


RecalboxSystem::RecalboxSystem() {
}

RecalboxSystem *RecalboxSystem::instance = NULL;

RecalboxSystem *RecalboxSystem::getInstance() {
    if (RecalboxSystem::instance == NULL) {
        RecalboxSystem::instance = new RecalboxSystem();
    }
    return RecalboxSystem::instance;
}

unsigned long RecalboxSystem::getFreeSpaceGB(std::string mountpoint) {
    struct statvfs fiData;
    const char *fnPath = mountpoint.c_str();
    int free = 0;
    if ((statvfs(fnPath, &fiData)) >= 0) {
        free = (fiData.f_bfree * fiData.f_bsize) / (1024 * 1024 * 1024);
    }
    return free;
}

std::string RecalboxSystem::getFreeSpaceInfo() {
    struct statvfs fiData;
    std::string sharePart = Settings::getInstance()->getString("SharePartition");
    if (sharePart.size() > 0) {
        const char *fnPath = sharePart.c_str();
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

bool RecalboxSystem::isFreeSpaceLimit() {
    std::string sharePart = Settings::getInstance()->getString("SharePartition");
    if (sharePart.size() > 0) {
        return getFreeSpaceGB(sharePart) < 2;
    } else {
        return "ERROR";
    }

}

std::string RecalboxSystem::getVersion() {
    std::string version = Settings::getInstance()->getString("VersionFile");
    if (version.size() > 0) {
        std::ifstream ifs(version);

        if (ifs.good()) {
            std::string contents;
            std::getline(ifs, contents);
            return contents;
        }
    }
    return "";
}

bool RecalboxSystem::needToShowVersionMessage() {
    std::string versionFile = Settings::getInstance()->getString("LastVersionFile");
    if (versionFile.size() > 0) {
        std::ifstream lvifs(versionFile);
        if (lvifs.good()) {
            std::string lastVersion;
            std::getline(lvifs, lastVersion);
            std::string currentVersion = getVersion();
            if (lastVersion == currentVersion) {
                return false;
            }
        }
    }
    return true;
}

bool RecalboxSystem::versionMessageDisplayed() {
    std::string versionFile = Settings::getInstance()->getString("LastVersionFile");
    std::string currentVersion = getVersion();
    std::ostringstream oss;
    oss << "echo " << currentVersion << " > " << versionFile;
    if (system(oss.str().c_str())) {
        LOG(LogWarning) << "Error executing " << oss.str().c_str();
        return false;
    } else {
        LOG(LogInfo) << "Version message displayed ok";
        return true;
    }

    return false;
}

std::string RecalboxSystem::getVersionMessage() {
    std::string versionMessageFile = Settings::getInstance()->getString("VersionMessage");
    if (versionMessageFile.size() > 0) {
        std::ifstream ifs(versionMessageFile);

        if (ifs.good()) {
            std::string contents((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());
            return contents;
        }
    }
    return "";

}

bool RecalboxSystem::setAudioOutputDevice(std::string device) {
    int commandValue = -1;
    int returnValue = false;

    if (device == "auto") {
        commandValue = 0;
    } else if (device == "jack") {
        commandValue = 1;
    } else if (device == "hdmi") {
        commandValue = 2;
    } else {
        LOG(LogWarning) << "Unable to find audio output device to use !";
    }

    if (commandValue != -1) {
        std::ostringstream oss;
        oss << "amixer cset numid=3 " << commandValue;
        std::string command = oss.str();
        LOG(LogInfo) << "Launching " << command;
        if (system(command.c_str())) {
            LOG(LogWarning) << "Error executing " << command;
            returnValue = false;
        } else {
            LOG(LogInfo) << "Audio output device set to : " << device;
            returnValue = true;
        }
    }
    return returnValue;
}


bool RecalboxSystem::setOverscan(bool enable) {

    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "overscan";
    if (enable) {
        oss << " " << "enable";
    } else {
        oss << " " << "disable";
    }
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if (system(command.c_str())) {
        LOG(LogWarning) << "Error executing " << command;
        return false;
    } else {
        LOG(LogInfo) << "Overscan set to : " << enable;
        return true;
    }

}

bool RecalboxSystem::setOverclock(std::string mode) {
    if (mode != "") {
        std::ostringstream oss;
        oss << Settings::getInstance()->getString("RecalboxSettingScript") << " "
        << "overclock" << " " << mode;
        std::string command = oss.str();
        LOG(LogInfo) << "Launching " << command;
        if (system(command.c_str())) {
            LOG(LogWarning) << "Error executing " << command;
            return false;
        } else {
            LOG(LogInfo) << "Overclocking set to " << mode;
            return true;
        }
    }

    return false;
}


bool RecalboxSystem::updateSystem() {
    std::string updatecommand = Settings::getInstance()->getString("UpdateCommand");
    if (updatecommand.size() > 0) {
        int exitcode = system(updatecommand.c_str());
        return exitcode == 0;
    }
    return false;
}

bool RecalboxSystem::ping() {
    std::string updateserver = Settings::getInstance()->getString("UpdateServer");
    std::string s("ping -c 1 " + updateserver);
    int exitcode = system(s.c_str());
    return exitcode == 0;
}

bool RecalboxSystem::canUpdate() {
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "canupdate";
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if (system(command.c_str()) == 0) {
        LOG(LogInfo) << "Can update ";
        return true;
    } else {
        LOG(LogInfo) << "Cannot update ";
        return false;
    }
}

bool RecalboxSystem::launchKodi(Window *window) {

    LOG(LogInfo) << "Attempting to launch kodi...";


    AudioManager::getInstance()->deinit();
    VolumeControl::getInstance()->deinit();

    window->deinit();

    std::string command = "/recalbox/scripts/kodilauncher.sh";
    int exitCode = system(command.c_str());

    window->init();
    VolumeControl::getInstance()->init();
    AudioManager::getInstance()->resumeMusic();
    window->normalizeNextUpdate();

    return exitCode == 0;

}

bool RecalboxSystem::enableWifi(std::string ssid, std::string key) {
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " "
    << "wifi" << " "
    << "enable" << " \""
    << ssid << "\" \"" << key << "\"";
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if (system(command.c_str()) == 0) {
        LOG(LogInfo) << "Wifi enabled ";
        return true;
    } else {
        LOG(LogInfo) << "Cannot enable wifi ";
        return false;
    }
}

bool RecalboxSystem::disableWifi() {
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " "
    << "wifi" << " "
    << "disable";
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if (system(command.c_str()) == 0) {
        LOG(LogInfo) << "Wifi disabled ";
        return true;
    } else {
        LOG(LogInfo) << "Cannot disable wifi ";
        return false;
    }
}


std::string RecalboxSystem::getRecalboxConfig(std::string key) {
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxConfigScript") << " "
    << " -command load "
    << " -key " << key;
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);

    return result;
}

bool RecalboxSystem::setRecalboxConfig(std::string key, std::string value) {
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxConfigScript") << " "
    << " -command save"
    << " -key " << key
    << " -value " << value;
    std::string command = oss.str();
    LOG(LogInfo) << "Launching " << command;
    if (system(command.c_str()) == 0) {
        LOG(LogInfo) << key << " saved in recalbox.conf";
        return true;
    } else {
        LOG(LogInfo) << "Cannot save " << key << " in recalbox.conf";
        return false;
    }
}


bool RecalboxSystem::reboot() {
    bool success = system("touch /tmp/reboot.please") == 0;
    SDL_Event *quit = new SDL_Event();
    quit->type = SDL_QUIT;
    SDL_PushEvent(quit);
    return success;
}

bool RecalboxSystem::shutdown() {
    bool success = system("touch /tmp/shutdown.please") == 0;
    SDL_Event *quit = new SDL_Event();
    quit->type = SDL_QUIT;
    SDL_PushEvent(quit);
    return success;
}

std::string RecalboxSystem::getIpAdress() {
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    void *tmpAddrPtr = NULL;

    std::string result = "NOT CONNECTED";
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            if (std::string(ifa->ifa_name).find("eth") != std::string::npos || std::string(ifa->ifa_name).find("wlan") != std::string::npos) {
                result = std::string(addressBuffer);
            }
        }
    }
    // Seeking for ipv6 if no IPV4
    if (result.compare("NOT CONNECTED") == 0) {
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
                // is a valid IP6 Address
                tmpAddrPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
                printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
                if (std::string(ifa->ifa_name).find("eth") != std::string::npos || std::string(ifa->ifa_name).find("wlan") != std::string::npos) {
                    return std::string(addressBuffer);
                }
            }
        }
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    return result;
}

std::vector<std::string> *RecalboxSystem::scanBluetooth() {
    std::vector<std::string> *res = new std::vector<std::string>();
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "hcitoolscan";
    FILE *pipe = popen(oss.str().c_str(), "r");
    char line[1024];

    if (pipe == NULL) {
        return NULL;
    }

    while (fgets(line, 1024, pipe)) {
        strtok(line, "\n");
        res->push_back(std::string(line));
    }
    pclose(pipe);

    return res;
}

bool RecalboxSystem::pairBluetooth(std::string &controller) {
    std::ostringstream oss;
    oss << Settings::getInstance()->getString("RecalboxSettingScript") << " " << "hiddpair" << " " << controller;
    int exitcode = system(oss.str().c_str());
    return exitcode == 0;
}
