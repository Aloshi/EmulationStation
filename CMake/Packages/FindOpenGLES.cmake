#snapped from: https://bitbucket.org/sinbad/ogre/src/0bba4f7cdb95/CMake/Packages/FindOpenGLES.cmake?at=default
#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES_FOUND        - system has OpenGLES
#  OPENGLES_INCLUDE_DIR  - the GL include directory
#  OPENGLES_LIBRARIES    - Link these to use OpenGLES

IF (WIN32)
  IF (CYGWIN)

    FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h )

    FIND_LIBRARY(OPENGLES_gl_LIBRARY libgles_cm )

  ELSE (CYGWIN)

    IF(MSVC)
      #The user hast to provide this atm. GLES can be emulated via Desktop OpenGL
      #using the ANGLE project found at: http://code.google.com/p/angleproject/
      SET (OPENGLES_gl_LIBRARY import32 CACHE STRING "OpenGL ES 1.x library for win32")
    ENDIF(MSVC)

  ENDIF (CYGWIN)

ELSE (WIN32)

  IF (APPLE)

        create_search_paths(/Developer/Platforms)
        findpkg_framework(OpenGLES)
    set(OPENGLES_gl_LIBRARY "-framework OpenGLES")

  ELSE(APPLE)

    IF (DEFINED BCMHOST)
      FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h
        /opt/vc/include
        NO_DEFAULT_PATH
      )

      FIND_LIBRARY(OPENGLES_gl_LIBRARY
        NAMES brcmGLESv2
        PATHS /opt/vc/lib
      )

    ELSE (DEFINED BCMHOST)

      FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h
        /usr/openwin/share/include
        /opt/graphics/OpenGL/include /usr/X11R6/include
        /usr/include
      )

      FIND_LIBRARY(OPENGLES_gl_LIBRARY
        NAMES GLES_CM GLESv1_CM
        PATHS /opt/graphics/OpenGL/lib
              /usr/openwin/lib
              /usr/shlib /usr/X11R6/lib
              /usr/lib
      )
    ENDIF (DEFINED BCMHOST)

    # On Unix OpenGL most certainly always requires X11.
    # Feel free to tighten up these conditions if you don't 
    # think this is always true.

    IF (OPENGLES_gl_LIBRARY)
      IF(NOT X11_FOUND)
        INCLUDE(FindX11)
      ENDIF(NOT X11_FOUND)
      IF (X11_FOUND)
        SET (OPENGLES_LIBRARIES ${X11_LIBRARIES})
      ENDIF (X11_FOUND)
    ENDIF (OPENGLES_gl_LIBRARY)

  ENDIF(APPLE)
ENDIF (WIN32)

SET( OPENGLES_FOUND "NO" )
IF(OPENGLES_gl_LIBRARY)

    SET( OPENGLES_LIBRARIES ${OPENGLES_gl_LIBRARY} ${OPENGLES_LIBRARIES})

    SET( OPENGLES_FOUND "YES" )

ENDIF(OPENGLES_gl_LIBRARY)

MARK_AS_ADVANCED(
  OPENGLES_INCLUDE_DIR
  OPENGLES_gl_LIBRARY
)
