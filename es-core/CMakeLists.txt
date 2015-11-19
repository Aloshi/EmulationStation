project("core")

set(CORE_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/AsyncHandle.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/GuiComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/HelpStyle.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/HttpReq.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageIO.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/InputConfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Log.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Music.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/platform.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sound.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/ThemeData.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Util.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/Window.h

	# Animations
	${CMAKE_CURRENT_SOURCE_DIR}/src/animations/Animation.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/animations/AnimationController.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/animations/LambdaAnimation.h

	# GuiComponents
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/AnimatedImageComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/BusyComponent.h
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
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScrollableContainer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/SliderComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/SwitchComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextComponent.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextEditComponent.h

	# Guis
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiDetectDevice.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiInputConfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMsgBox.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiTextEditPopup.h

	# Resources
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/Font.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ResourceManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/SVGResource.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/TextureResource.h

	# Embedded assets (needed by ResourceManager)
	${emulationstation-all_SOURCE_DIR}/data/Resources.h
)

set(CORE_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/AudioManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/GuiComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HelpStyle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HttpReq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ImageIO.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/InputConfig.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Log.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Music.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/platform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_draw_gl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer_init_sdlgl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Settings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sound.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ThemeData.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Util.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp

	# Animations
	${CMAKE_CURRENT_SOURCE_DIR}/src/animations/AnimationController.cpp

	# GuiComponents
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/AnimatedImageComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/BusyComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/ButtonComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentGrid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/ComponentList.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/DateTimeComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/HelpComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/ImageComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/MenuComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/NinePatchComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/ScrollableContainer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/SliderComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/SwitchComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextComponent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/components/TextEditComponent.cpp

	# Guis
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiDetectDevice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiInputConfig.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiMsgBox.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/guis/GuiTextEditPopup.cpp

	# Resources
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/Font.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/ResourceManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/SVGResource.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/resources/TextureResource.cpp
)

set(EMBEDDED_ASSET_SOURCES
    ${emulationstation-all_SOURCE_DIR}/data/ResourceUtil.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/splash_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/window_icon_256_png.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/button_png.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/button_filled_png.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/textinput_ninepatch_png.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/textinput_ninepatch_active_png.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/frame_png.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/scroll_gradient_png.cpp

    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_a_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_b_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_x_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_y_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_l_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_r_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_l2_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_r2_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_l3_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_r3_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_start_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_select_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_button_hotkey_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_up_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_down_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_left_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_right_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_updown_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_leftright_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_dpad_all_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_joystick_left_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/help_joystick_up_svg.cpp


    ${emulationstation-all_SOURCE_DIR}/data/converted/ubuntu_condensed_ttf.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/fontawesome_webfont_ttf.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/arrow_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/option_arrow_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/checkbox_checked_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/checkbox_unchecked_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/star_filled_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/star_unfilled_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/on_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/off_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/fav_add_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/fav_remove_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/slider_knob_svg.cpp

    ${emulationstation-all_SOURCE_DIR}/data/converted/busy_0_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/busy_1_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/busy_2_svg.cpp
    ${emulationstation-all_SOURCE_DIR}/data/converted/busy_3_svg.cpp
)

list(APPEND CORE_SOURCES ${EMBEDDED_ASSET_SOURCES})

include_directories(${COMMON_INCLUDE_DIRS})
add_library(es-core STATIC ${CORE_SOURCES} ${CORE_HEADERS} src/RecalboxConf.cpp src/RecalboxConf.h)
target_link_libraries(es-core ${COMMON_LIBRARIES})
