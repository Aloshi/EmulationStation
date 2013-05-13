//the Makefiles define these via command line
//#define _RPI_
//#define _DESKTOP_

#ifdef _RPI_
	#define GLHEADER <GLES/gl.h>
#endif


#ifdef _DESKTOP_
	//why the hell this naming inconsistency exists is well beyond me
	#ifdef _WIN32
		#define sleep Sleep
	#endif

	#define GLHEADER <SDL_opengl.h>
#endif

#include <string>

std::string getHomePath();