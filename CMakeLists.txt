cmake_minimum_required(VERSION 2.6)

project(emulationstation)

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

#finding necessary packages
#-------------------------------------------------------------------------------
if(${GLSystem} MATCHES "Desktop OpenGL")
    find_package(OpenGL REQUIRED)
else()
    find_package(OpenGLES REQUIRED)
endif()
find_package(Freetype REQUIRED)
find_package(FreeImage REQUIRED)
find_package(SDL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Eigen3 REQUIRED)

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(ALSA REQUIRED)
endif()

#-------------------------------------------------------------------------------
#set up compiler flags and excutable names
if(DEFINED BCMHOST)
    add_definitions(-D_RPI_)
endif()

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2") #support C++11 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")  #strip binary
endif()

if(${GLSystem} MATCHES "Desktop OpenGL")
    add_definitions(-DUSE_OPENGL_DESKTOP)
else()
    add_definitions(-DUSE_OPENGL_ES)
endif()

add_definitions(-DEIGEN_DONT_ALIGN)

#-------------------------------------------------------------------------------
#add include directories
set(ES_INCLUDE_DIRS
    ${FREETYPE_INCLUDE_DIRS}
    ${FreeImage_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND ES_INCLUDE_DIRS
        ${ALSA_INCLUDE_DIRS}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND ES_INCLUDE_DIRS
        "/opt/vc/include"
        "/opt/vc/include/interface/vcos"
        "/opt/vc/include/interface/vmcs_host/linux"
        "/opt/vc/include/interface/vcos/pthreads"
    )
else()
    if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND ES_INCLUDE_DIRS
            ${OPENGL_INCLUDE_DIR}
        )
    else()
        LIST(APPEND ES_INCLUDE_DIRS
            ${OPENGLES_INCLUDE_DIR}
        )
    endif()
endif()

#-------------------------------------------------------------------------------
#define basic sources and headers
set(ES_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EmulationStation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FolderData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GuiComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageIO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MathExp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sound.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VolumeControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/XMLReader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/AnimationComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentListComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ImageComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScrollableContainer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SliderComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SwitchComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextListComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ThemeComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiDetectDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiFastSelect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiGameList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiInputConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiMenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiSettingsMenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pugiXML/pugiconfig.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pugiXML/pugixml.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ResourceManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/TextureResource.h
	${CMAKE_CURRENT_SOURCE_DIR}/data/Resources.h
)
set(ES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FolderData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GuiComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MathExp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_draw_gl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VolumeControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/XMLReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/AnimationComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentListComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ImageComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScrollableContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SliderComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SwitchComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ThemeComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiDetectDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiFastSelect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiGameList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiInputConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/GuiSettingsMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pugiXML/pugixml.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ResourceManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/TextureResource.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/data/ResourceUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/ES_logo_16_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/ES_logo_32_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/bar_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/corner_png.cpp
)

SOURCE_GROUP(resources FILES ResourceUtil.cpp)

#add open gl specific sources
if(${GLSystem} MATCHES "Desktop OpenGL")
    LIST(APPEND ES_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init_sdlgl.cpp
    )
else()
    LIST(APPEND ES_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init_rpi.cpp
    )
endif()

#-------------------------------------------------------------------------------
#define OS specific sources and headers
if(MSVC)
    LIST(APPEND ES_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/EmulationStation.rc
    )
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

set(ES_LIBRARIES
    ${Boost_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${FreeImage_LIBRARIES}
	${SDL_LIBRARY}
    ${SDLMAIN_LIBRARY}
)

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND ES_LIBRARIES
        ${ALSA_LIBRARY}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND ES_LIBRARIES
        bcm_host
        EGL
        ${OPENGLES_LIBRARIES}
    )
else()
    if(MSVC)
        LIST(APPEND ES_LIBRARIES
            winmm
        )
    endif()
    if(${GLSystem} MATCHES "Desktop OpenGL")
        LIST(APPEND ES_LIBRARIES
            ${OPENGL_LIBRARIES}
        )
    else()
        LIST(APPEND ES_LIBRARIES
            ${OPENGLES_LIBRARIES}
        )
    endif()
endif()

#-------------------------------------------------------------------------------
#set up build directories
set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

#-------------------------------------------------------------------------------
#define target
include_directories(${ES_INCLUDE_DIRS})
add_executable(emulationstation ${ES_SOURCES} ${ES_HEADERS})
target_link_libraries(emulationstation ${ES_LIBRARIES})

#special properties for windows builds
if(MSVC)
    #show console in debug builds, but not in proper release builds
    #Note that up to CMake 2.8.10 this feature is broken: http://public.kitware.com/Bug/view.php?id=12566
    set_target_properties(emulationstation PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(emulationstation PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
    set_target_properties(emulationstation PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(emulationstation PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
    set_target_properties(emulationstation PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(emulationstation PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif()
