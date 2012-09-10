//the Makefiles define these via command line
//#define _RPI_
//#define _DESKTOP_


#ifdef _RPI_
	#define GLHEADER <GLES/gl.h>
#endif


#ifdef _DESKTOP_
	#define GLHEADER <GL/gl.h>
#endif
