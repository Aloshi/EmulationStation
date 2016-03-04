#pragma once

//the Makefile defines one of these:
//#define USE_OPENGL_ES
//#define USE_OPENGL_DESKTOP

#ifdef USE_OPENGL_ES
    #include <GLES/gl.h>
#endif

#ifdef USE_OPENGL_DESKTOP
    //why the hell this naming inconsistency exists is well beyond me
    #ifdef WIN32
        #define sleep Sleep
    #endif

    #include <SDL_opengl.h>
#endif
