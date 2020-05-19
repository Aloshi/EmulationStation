# FindOpenGLES
# ------------
# Finds the OpenGLES2 library
#
# This will define the following variables::
#
# OPENGLES2_FOUND - system has OpenGLES
# OPENGLES2_INCLUDE_DIRS - the OpenGLES include directory
# OPENGLES2_LIBRARIES - the OpenGLES libraries

if(NOT HINT_GLES_LIBNAME)
 set(HINT_GLES_LIBNAME GLESv2)
endif()

find_path(OPENGLES2_INCLUDE_DIR GLES2/gl2.h
    PATHS "${CMAKE_FIND_ROOT_PATH}/usr/include"
    HINTS ${HINT_GLES_INCDIR}
)

find_library(OPENGLES2_gl_LIBRARY
    NAMES ${HINT_GLES_LIBNAME}
    HINTS ${HINT_GLES_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGLES2
            REQUIRED_VARS OPENGLES2_gl_LIBRARY OPENGLES2_INCLUDE_DIR)


if(OPENGLES2_FOUND)
    set(OPENGLES2_LIBRARIES ${OPENGLES2_gl_LIBRARY})
    set(OPENGLES2_INCLUDE_DIRS ${OPENGLES2_INCLUDE_DIR})
    mark_as_advanced(OPENGLES2_INCLUDE_DIR OPENGLES2_gl_LIBRARY)
endif()

