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
find_package(SDL2 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem regex date_time)
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
#set up _WIN32_WINNT variable (used by boost::asio) on Windows
macro(get_WIN32_WINNT version)
    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

        set(${version} "0x${ver}")
    endif()
endmacro()

if(WIN32)
    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-attributes") #support C++11 for std::, optimize
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
    ${SDL2_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AsyncHandle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EmulationStation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileSorts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GuiComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HttpReq.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageIO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MathExp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MetaData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlatformId.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ScraperCmdLine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sound.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThemeData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VolumeControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/XMLReader.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/AsyncReqComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ButtonComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentGrid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/DateTimeComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/HelpComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/IList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ImageComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ImageGridComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/MenuComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/NinePatchComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/OptionListComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/RatingComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScraperSearchComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScrollableContainer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SliderComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SwitchComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextEditComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextListComponent.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiDetectDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiFastSelect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMetaDataEd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMsgBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiGameScraper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiGamelistOptions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiInputConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiSettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiScraperMulti.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiScraperStart.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiTextEditPopup.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrapers/Scraper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrapers/GamesDBScraper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrapers/TheArchiveScraper.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/pugiXML/pugiconfig.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pugiXML/pugixml.hpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/nanosvg/nanosvg.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/nanosvg/nanosvgrast.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/BasicGameListView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/DetailedGameListView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/IGameListView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/ISimpleGameListView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/GridGameListView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/SystemView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/ViewController.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/animations/Animation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/animations/AnimationController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/animations/LambdaAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/animations/LaunchAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/animations/MoveCameraAnimation.h

    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/Font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ResourceManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/SVGResource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/TextureResource.h
	${CMAKE_CURRENT_SOURCE_DIR}/data/Resources.h
)
set(ES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileSorts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GuiComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HttpReq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MathExp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MetaData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_draw_gl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ScraperCmdLine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThemeData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VolumeControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/XMLReader.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/AsyncReqComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ButtonComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentGrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/DateTimeComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/HelpComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ImageComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/MenuComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/NinePatchComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/RatingComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScraperSearchComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScrollableContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SliderComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/SwitchComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextEditComponent.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiDetectDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiFastSelect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMetaDataEd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMsgBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiGameScraper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiGamelistOptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiInputConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiSettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiScraperMulti.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiScraperStart.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiTextEditPopup.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrapers/Scraper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrapers/GamesDBScraper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrapers/TheArchiveScraper.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/pugiXML/pugixml.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nanosvg/nanosvg_impl.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/Font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ResourceManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/SVGResource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/TextureResource.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/BasicGameListView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/DetailedGameListView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/IGameListView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/ISimpleGameListView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/gamelist/GridGameListView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/SystemView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/ViewController.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/animations/AnimationController.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/data/ResourceUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/ES_logo_16_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/ES_logo_32_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/button_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/button_filled_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/textinput_ninepatch_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/frame_png.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/scroll_gradient_png.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_a_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_b_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_x_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_y_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_l_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_r_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_start_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_button_select_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_up_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_down_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_left_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_right_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_updown_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_leftright_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/help_dpad_all_svg.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/opensans_hebrew_condensed_regular_ttf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/opensans_hebrew_condensed_light_ttf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/arrow_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/option_arrow_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/checkbox_checked_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/checkbox_unchecked_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/star_filled_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/star_unfilled_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/on_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/off_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/fav_add_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/fav_remove_svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/converted/slider_knob_svg.cpp
)

#SOURCE_GROUP(resources FILES ResourceUtil.cpp)

#add open gl specific sources
#if(${GLSystem} MATCHES "Desktop OpenGL")
    LIST(APPEND ES_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init_sdlgl.cpp
    )
#else()
#    LIST(APPEND ES_SOURCES
#		${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init_rpi.cpp
#    )
#endif()

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
	${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${CURL_LIBRARIES}
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
