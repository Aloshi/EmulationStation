cmake_minimum_required(VERSION 2.8)

project(emulationstation-all)

#-------------------------------------------------------------------------------
#add local find scripts to CMAKE path
LIST(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages
)

#-------------------------------------------------------------------------------
#set up OpenGL system variable
set(GLSystem "Desktop OpenGL" CACHE STRING "The OpenGL system to be used")
set_property(CACHE GLSystem PROPERTY STRINGS "Desktop OpenGL" "OpenGL ES")

#-------------------------------------------------------------------------------
#check if we're running on Raspberry Pi
MESSAGE("Looking for bcm_host.h")
if(EXISTS "/opt/vc/include/bcm_host.h")
    MESSAGE("bcm_host.h found")
    set(BCMHOST found)
    set(GLSystem "OpenGL ES")
else()
    MESSAGE("bcm_host.h not found")
endif()

#-------------------------------------------------------------------------------
#check if we're running on olinuxino
MESSAGE("Looking for libMali.so")
if(EXISTS "/usr/lib/libMali.so")
    MESSAGE("libMali.so found")
    set(GLSystem "OpenGL ES")
else()
    MESSAGE("libMali.so not found")
endif()

#finding necessary packages
#-------------------------------------------------------------------------------
if(${GLSystem} MATCHES "Desktop OpenGL")
    find_package(OpenGL REQUIRED)
else()
    find_package(OpenGLES REQUIRED)
endif()
find_package(Freetype REQUIRED)
find_package(FreeImage REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem date_time locale)
find_package(Eigen3 REQUIRED)
find_package(CURL REQUIRED)

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(ALSA REQUIRED)
endif()

#-------------------------------------------------------------------------------
#set up compiler flags and excutable names
if(DEFINED BCMHOST)
    add_definitions(-D_RPI_)
endif()

#-------------------------------------------------------------------------------

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    #check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 4.7)
            message(SEND_ERROR "You need at least G++ 4.7 to compile EmulationStation!")
    endif()

    #set up compiler flags for GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-attributes -O3") #support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")  #-s = strip binary
endif()

if(${GLSystem} MATCHES "Desktop OpenGL")
    add_definitions(-DUSE_OPENGL_DESKTOP)
else()
    add_definitions(-DUSE_OPENGL_ES)
endif()

add_definitions(-DEIGEN_DONT_ALIGN)

#-------------------------------------------------------------------------------
#add include directories
set(COMMON_INCLUDE_DIRS
    ${FREETYPE_INCLUDE_DIRS}
    ${FreeImage_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/es-core/src
)

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_INCLUDE_DIRS
        ${ALSA_INCLUDE_DIRS}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND COMMON_INCLUDE_DIRS
        "/opt/vc/include"
        "/opt/vc/include/interface/vcos"
        "/opt/vc/include/interface/vmcs_host/linux"
        "/opt/vc/include/interface/vcos/pthreads"
    )
else()
    if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGL_INCLUDE_DIR}
        )
    else()
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGLES_INCLUDE_DIR}
        )
    endif()
endif()

#-------------------------------------------------------------------------------
#define libraries and directories
if(DEFINED BCMHOST)
    link_directories(
        ${Boost_LIBRARY_DIRS}
        "/opt/vc/lib"
    )
else()
    link_directories(
        ${Boost_LIBRARY_DIRS}
    )
endif()

set(COMMON_LIBRARIES
    ${Boost_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${FreeImage_LIBRARIES}
	${SDL2_LIBRARY}
    ${CURL_LIBRARIES}
    pugixml
    nanosvg
)

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_LIBRARIES
        ${ALSA_LIBRARY}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND COMMON_LIBRARIES
        bcm_host
        EGL
        ${OPENGLES_LIBRARIES}
    )
else()
    if(MSVC)
        LIST(APPEND COMMON_LIBRARIES
            winmm
        )
    endif()
    if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND COMMON_LIBRARIES
            ${OPENGL_LIBRARIES}
        )
    else()
        LIST(APPEND COMMON_LIBRARIES
            EGL
            ${OPENGLES_LIBRARIES}
        )
    endif()
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
