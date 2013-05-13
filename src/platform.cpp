#include "platform.h"
#include <stdlib.h>

std::string getHomePath()
{
	#ifdef _WIN32
		return "C:\\";
	#else
		const char* home = getenv("HOME");
		if(home == NULL)
			return "";
		else
			return home;
	#endif
}
