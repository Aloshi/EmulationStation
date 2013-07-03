#include "platform.h"
#include <stdlib.h>
#include <boost/filesystem.hpp>


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

			for(unsigned int i = 0; i < homePath.length(); i++)
				if(homePath[i] == '\\')
					homePath[i] = '/';
		}
	}
#else
	if (homePath.empty()) {
		homePath = "~";
	}
#endif

	//convert path to generic directory seperators
	boost::filesystem::path genericPath(homePath);
	return genericPath.generic_string();
}
