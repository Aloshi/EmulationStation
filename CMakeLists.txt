cmake_minimum_required(VERSION 2.8)

option(GLES "Set to ON if targeting Embedded OpenGL" ${GLES})
option(GL "Set to ON if targeting Desktop OpenGL" ${GL})
option(RPI "Set to ON to enable the Raspberry PI video player (omxplayer) and audio options" ${RPI})
option(CEC "Set to ON to enable CEC" ${CEC})
option(PROFILING "Set to ON to enable profiling" ${PROFILING})

# GLES implementation overrides
option(USE_MESA_GLES "Set to ON to select the MESA OpenGL ES driver" ${USE_MESA_GLES})
option(USE_GLES1 "Set to ON to force usage of the OpenGLES v1 renderer" ${USE_GLES1})

# OpenGL library preference (https://cmake.org/cmake/help/git-stage/policy/CMP0072.html)
# Set it to OLD to appease older proprietary drivers without libglvnd support
if(POLICY CMP0072)
   cmake_policy(SET CMP0072 OLD)
endif()

project(emulationstation-all)

#-------------------------------------------------------------------------------
#add local find scripts to CMAKE path
LIST(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages
)

# default GLES2/GLES library, can be overriden depending on platform
set(HINT_GLES_LIBNAME GLESv2)

#set up OpenGL system variable
if(GLES)
    set(GLSystem "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
elseif(GL)
    set(GLSystem "Desktop OpenGL" CACHE STRING "The OpenGL system to be used")

# check if the MESA GLES library is requested
elseif(USE_MESA_GLES)
    set(GLSystem "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")

#check if we're running on Raspberry Pi (legacy drivers)
elseif(EXISTS "${CMAKE_FIND_ROOT_PATH}/opt/vc/include/bcm_host.h")
    set(BCMHOST found)
    set(GLSystem "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
    set(HINT_GLES_INCDIR "/opt/vc/include")
    set(HINT_GLES_LIBDIR "/opt/vc/lib")
    set(HINT_GLES_LIBNAME brcmGLESv2)
    set(GLES_EXTRA_LIBRARIES bcm_host)

#check if we're running on OSMC Vero4K[+]
elseif(EXISTS "${CMAKE_FIND_ROOT_PATH}/opt/vero3/lib/libMali.so")
    set(VERO4K found)
    add_definitions(-D_VERO4K_)
    set(GLSystem "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
    set(HINT_GLES_INCDIR "/opt/vero3/include")
    set(HINT_GLES_LIBDIR "/opt/vero3/lib")
    set(HINT_GLES_LIBNAME Mali)
    set(GLES_EXTRA_LIBRARIES EGL)

#check if we're running on olinuxino / odroid / etc
elseif(
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/libMali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/libmali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/libMali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/mali-egl/libmali.so")
        set(GLSystem "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
else()
    set(GLSystem "Desktop OpenGL" CACHE STRING "The OpenGL system to be used")
endif(GLES)

set_property(CACHE GLSystem PROPERTY STRINGS "Desktop OpenGL" "Embedded OpenGL")

if(${GLSystem} MATCHES "Desktop OpenGL")
    find_package(OpenGL REQUIRED)
    add_definitions(-DUSE_OPENGL_14)
else()
    if(NOT USE_GLES1)
        find_package(OpenGLES2 QUIET REQUIRED)
        message(STATUS "Found GLESv2: ${OPENGLES2_LIBRARIES}")
        set(OPENGLES_LIBRARIES ${OPENGLES2_LIBRARIES})
        set(OPENGLES_INCLUDE_DIRS ${OPENGLES2_INCLUDE_DIRS})
        add_definitions(-DUSE_OPENGLES_20)
    else()
        find_package(OpenGLES QUIET REQUIRED)
        message(STATUS "Found GLES: ${OPENGLES_LIBRARIES}")
        add_definitions(-DUSE_OPENGLES_10)
    endif()
endif()

find_package(Freetype REQUIRED)
find_package(FreeImage REQUIRED)
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)
find_package(VLC REQUIRED)
find_package(RapidJSON REQUIRED)

#add libCEC support
if(CEC)
    find_package(libCEC REQUIRED)
endif()

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(ALSA REQUIRED)
endif()

#-------------------------------------------------------------------------------
#set up compiler flags and excutable names
if(DEFINED BCMHOST OR RPI)
    add_definitions(-D_RPI_)
endif()

if(DEFINED libCEC_FOUND)
    add_definitions(-DHAVE_LIBCEC)
endif()

if(PROFILING)
    add_definitions(-DUSE_PROFILING)
endif()

#-------------------------------------------------------------------------------

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation

    # Set the start-up project to in VS to 'emulationstation'
    set(VS_STARTUP_PROJECT "emulationstation")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    #check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 4.7)
            message(SEND_ERROR "You need at least G++ 4.7 to compile EmulationStation!")
    endif()

    #set up compiler flags for GCC
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-attributes -O0") #support C++11 for std::, optimize
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-attributes -O2") #support C++11 for std::, optimize
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2")  #-s = strip binary
endif()
endif()

# Enable additional defines for the Debug build configuration
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")


#-------------------------------------------------------------------------------
#add include directories
set(COMMON_INCLUDE_DIRS
    ${FREETYPE_INCLUDE_DIRS}
    ${FreeImage_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${VLC_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/es-core/src
)

if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGL_INCLUDE_DIRS}
        )
else()
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGLES_INCLUDE_DIRS}
        )
endif()

#add libCEC_INCLUDE_DIR
if(DEFINED libCEC_FOUND)
    LIST(APPEND COMMON_INCLUDE_DIRS
        ${libCEC_INCLUDE_DIR}
    )
endif()

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_INCLUDE_DIRS
        ${ALSA_INCLUDE_DIRS}
    )
endif()

#-------------------------------------------------------------------------------
#define libraries and directories

set(COMMON_LIBRARIES
    ${FREETYPE_LIBRARIES}
    ${FreeImage_LIBRARIES}
    ${SDL2_LIBRARY}
    ${CURL_LIBRARIES}
    ${VLC_LIBRARIES}
    pugixml
    nanosvg
)

#add libCEC_LIBRARIES
if(DEFINED libCEC_FOUND)
if(DEFINED BCMHOST)
    LIST(APPEND COMMON_LIBRARIES
        vchiq_arm
    )
endif()
    LIST(APPEND COMMON_LIBRARIES
        dl
        ${libCEC_LIBRARIES}
    )
endif()

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_LIBRARIES
        ${ALSA_LIBRARY}
    )
endif()

if(MSVC)
    LIST(APPEND COMMON_LIBRARIES
        winmm
    )
endif()

if(DEFINED HINT_GLES_LIBDIR)
    link_directories("${HINT_GLES_LIBDIR}")
endif()

if(${GLSystem} MATCHES "Desktop OpenGL")
    LIST(APPEND COMMON_LIBRARIES
        ${OPENGL_LIBRARIES}
    )
else()
    LIST(APPEND COMMON_LIBRARIES
        ${OPENGLES_LIBRARIES} ${GLES_EXTRA_LIBRARIES}
    )
endif()


#-------------------------------------------------------------------------------
# set up build directories
set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)


#-------------------------------------------------------------------------------
# add each component

add_subdirectory("external")
add_subdirectory("es-core")
add_subdirectory("es-app")
