/* 
 * File:   SystemInfo.h
 * Author: matthieu
 *
 * Created on 29 novembre 2014, 03:15
 */

#ifndef SYSTEMINFO_H
#define	SYSTEMINFO_H
#include <string>


class SystemInfo {
public:
    
    
    static SystemInfo * getInstance();

    unsigned long getFreeSpaceGB(std::string mountpoint);
    std::string getFreeSpaceInfo();
    bool isFreeSpaceLimit();
    std::string getVersion();

private:
    static SystemInfo * instance;
    SystemInfo();

};

#endif	/* SYSTEMINFO_H */

