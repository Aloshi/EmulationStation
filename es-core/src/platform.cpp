#include "platform.h"
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <sys/statvfs.h>
#include <sstream>

#include <fstream>


std::string getHomePath()
{
	std::string homePath;

	// this should give you something like "/home/YOUR_USERNAME" on Linux and "C:\Users\YOUR_USERNAME\" on Windows
	const char * envHome = getenv("HOME");
	if(envHome != nullptr)
	{
		homePath = envHome;
	}

#ifdef WIN32
	// but does not seem to work for Windows XP or Vista, so try something else
	if (homePath.empty()) {
		const char * envDir = getenv("HOMEDRIVE");
		const char * envPath = getenv("HOMEPATH");
		if (envDir != nullptr && envPath != nullptr) {
			homePath = envDir;
			homePath += envPath;

			for(unsigned int i = 0; i < homePath.length(); i++)
				if(homePath[i] == '\\')
					homePath[i] = '/';
		}
	}
#endif

	// convert path to generic directory seperators
	boost::filesystem::path genericPath(homePath);
	return genericPath.generic_string();
}

int runShutdownCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -s -t 0");
#else // osx / linux
	return system("sudo shutdown -h now");
#endif
}

int runRestartCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -r -t 0");
#else // osx / linux
	return system("sudo shutdown -r now");
#endif
}

unsigned long getFreeSpaceGB(std::string mountpoint)
{
    struct statvfs fiData;
    const char * fnPath = mountpoint.c_str();
    unsigned long free = (fiData.f_bfree * fiData.f_bsize) / (1024*1024*1024);
    return free;
}

std::string getFreeSpaceInfo()
{
    struct statvfs fiData;
    const char * fnPath = std::string("/").c_str();
    if((statvfs(fnPath,&fiData)) < 0 ) {
            printf("Failed to stat %s\n", fnPath);
    } else {
            printf("Disk %s: \n", fnPath);
            printf("\tblock size: %u\n", fiData.f_bsize);
            printf("\ttotal no blocks: %i\n", fiData.f_blocks);
            printf("\tfree blocks: %i\n", fiData.f_bfree);
    }
    unsigned long total = (fiData.f_blocks * fiData.f_bsize) / (1024*1024*1024);
    unsigned long free = (fiData.f_bfree * fiData.f_bsize) / (1024*1024*1024);
    unsigned long used = total - free;
    unsigned long percent = used * 100 / total;
    
    std::ostringstream oss;
    oss << used << "GO/" << total << "GO (" << percent << "%)" ;
    return oss.str();
}

bool isFreeSpaceLimit()
{
    return getFreeSpaceGB("/") < 2;
}

std::string getVersion()
{
      std::ifstream ifs("");
}