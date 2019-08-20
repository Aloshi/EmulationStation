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

enum QuitMode
{
	QUIT = 0,
	RESTART = 1,
	SHUTDOWN = 2,
	REBOOT = 3
};

int runSystemCommand(const std::string& cmd_utf8); // run a utf-8 encoded in the shell (requires wstring conversion on Windows)
int quitES(QuitMode mode = QuitMode::QUIT);
void processQuitMode();

#endif // ES_CORE_PLATFORM_H
