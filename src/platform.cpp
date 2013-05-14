#include "platform.h"
#include <stdlib.h>

std::string getHomePath()
{
	//this gives you something like "/home/YOUR_USERNAME" on Linux and "C:\Users\YOUR_USERNAME\" on Windows
	const char* home = getenv("HOME");
	if(home == NULL)
		return "";
	else
		return home;
}
