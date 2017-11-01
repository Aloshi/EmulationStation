#pragma once
#ifndef ES_CORE_PLATFORM_H
#define ES_CORE_PLATFORM_H

#include <string>

//the Makefile defines one of these:
//#define USE_OPENGL_ES
//#define USE_OPENGL_DESKTOP

#ifdef USE_OPENGL_ES
	#define GLHEADER <GLES/gl.h>
#endif

#ifdef USE_OPENGL_DESKTOP
	//why the hell this naming inconsistency exists is well beyond me
	#ifdef WIN32
		#define sleep Sleep
	#endif

	#define GLHEADER <SDL_opengl.h>
#endif

std::string getHomePath();


int runShutdownCommand(); // shut down the system (returns 0 if successful)
int runRestartCommand(); // restart the system (returns 0 if successful)
int runSystemCommand(const std::string& cmd_utf8); // run a utf-8 encoded in the shell (requires wstring conversion on Windows)
int quitES(const std::string& filename);
void touch(const std::string& filename);

#endif // ES_CORE_PLATFORM_H
