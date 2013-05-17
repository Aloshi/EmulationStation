#include "platform.h"
#include <stdlib.h>


std::string getHomePath()
{
	std::string homePath;

	//this should give you something like "/home/YOUR_USERNAME" on Linux and "C:\Users\YOUR_USERNAME\" on Windows
	const char * envHome = getenv("HOME");
	if(envHome != nullptr) {
		homePath = envHome;
	}
#ifdef WIN32
	//but does not seem to work for Windwos XP or Vista, so try something else
	if (homePath.empty()) {
		const char * envDir = getenv("HOMEDRIVE");
		const char * envPath = getenv("HOMEPATH");
		if (envDir != nullptr && envPath != nullptr) {
			homePath = envDir;
			homePath += envPath;
		}
	}
#else
	if (homePath.empty()) {
		homePath = "~";
	}
#endif

	return homePath;
}
