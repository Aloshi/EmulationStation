# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES_FOUND        - system has OpenGLES
#  OPENGLES_INCLUDE_DIRS  - the GL include directory
#  OPENGLES_LIBRARIES    - Link these to use OpenGLES

if(NOT HINT_GLES_LIBNAME)
 set(HINT_GLES_LIBNAME GLESv1_CM)
endif()

if (WIN32)
    if(CYGWIN)
        find_path(OPENGLES_INCLUDE_DIR GLES/gl.h )
        find_library(OPENGLES_gl_LIBRARY libgles_cm )
  else(CYGWIN)
    if(MSVC)
      #The user has to provide this atm. GLES can be emulated via Desktop OpenGL
      #using the ANGLE project found at: http://code.google.com/p/angleproject/
      SET (OPENGLES_gl_LIBRARY import32 CACHE STRING "OpenGL ES 1.x library for win32")
    endif(MSVC)
endif(CYGWIN)
elseif(APPLE)
    create_search_paths(/Developer/Platforms)
    findpkg_framework(OpenGLES)
    set(OPENGLES_gl_LIBRARY "-framework OpenGLES")
else()
    find_path(OPENGLES_INCLUDE_DIR GLES/gl.h
              PATHS "${CMAKE_FIND_ROOT_PATH}/usr/include"
              HINTS "${HINT_GLES_INCDIR}"
    )

    find_library(OPENGLES_gl_LIBRARY
        NAMES ${HINT_GLES_LIBNAME}
        HINTS "${HINT_GLES_LIBDIR}"
    )
endif(WIN32)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES
    REQUIRED_VARS OPENGLES_gl_LIBRARY OPENGLES_INCLUDE_DIR)


if(OPENGLES_FOUND)
    set(OPENGLES_LIBRARIES ${OPENGLES_gl_LIBRARY})
    set(OPENGLES_INCLUDE_DIRS ${OPENGLES_INCLUDE_DIR})
    mark_as_advanced(OPENGLES_INCLUDE_DIR OPENGLES_gl_LIBRARY)
endif()
