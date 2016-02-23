#include "platform.h"
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <iostream>

#if defined(WIN32)
	#include <codecvt>
#elif defined(__linux__)
	#include <unistd.h>
	#include <sys/reboot.h>
#endif

namespace fs = boost::filesystem;

std::string getConfigDirectory()
{
    fs::path path;
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
    CHAR my_documents[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_PRESONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);
    path = fs::path(my_documents) / fs::path("EmulationStation");
#elif __APPLE__ && !defined(USE_XDG_OSX)
    const char* homePath = getenv("HOME");
    path = boost::filesystem::path(homePath);
    path /= fs::path("Library") / fs::path("Application Support") / fs::path("org.emulationstation.EmulationStation") ;
#else
    const char* envXdgConfig = getenv("XDG_CONFIG_HOME");
    if(envXdgConfig){
        path = fs::path(envXdgConfig);
    } else {
        const char* homePath = getenv("HOME");
        path = fs::path(homePath);
        path /= fs::path(".config");
    }
    path /= fs::path("emulationstation");
#endif
    return path.generic_string();
}

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
#if defined(WIN32)
	return system("shutdown -s -t 0");
#elif defined(__linux__)
	sync();
	return reboot(RB_POWER_OFF);
#else
	return system("sudo shutdown -h now");
#endif
}

int runRestartCommand()
{
#if defined(WIN32)
	return system("shutdown -r -t 0");
#elif defined(__linux__)
	sync();
	return reboot(RB_AUTOBOOT);
#else
	return system("sudo shutdown -r now");
#endif
}

int runSystemCommand(const std::string& cmd_utf8)
{
#ifdef WIN32
	// on Windows we use _wsystem to support non-ASCII paths
	// which requires converting from utf8 to a wstring
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::wstring wchar_str = converter.from_bytes(cmd_utf8);
	return _wsystem(wchar_str.c_str());
#else
	return system(cmd_utf8.c_str());
#endif
}
