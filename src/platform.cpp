#include "platform.h"
#include <stdlib.h>
#include <boost/filesystem.hpp>

std::string sHomePathOverride;

void setHomePathOverride(const std::string& path)
{
	// make it use generic directory separators
	sHomePathOverride = boost::filesystem::path(path).generic_string();
}

std::string getHomePath()
{
	if(!sHomePathOverride.empty())
		return sHomePathOverride;

	std::string homePath;

	// this should give you something like "/home/YOUR_USERNAME" on Linux and "C:\Users\YOUR_USERNAME\" on Windows
	const char * envHome = getenv("HOME");
	if(envHome != nullptr)
	{
		homePath = envHome;
	}

#ifdef WIN32
	// but does not seem to work for Windwos XP or Vista, so try something else
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
