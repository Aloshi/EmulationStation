#include "platform.h"

#ifdef _RPI_
	#include "Renderer_init_rpi.cpp"
#endif

#ifdef _DESKTOP_
	#include "Renderer_init_sdlgl.cpp"
#endif
