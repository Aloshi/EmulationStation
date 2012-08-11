EmulationStation
================

A simple front-end for emulators made with SDL, designed for controller navigation. Developed for use with the Raspberry Pi and RetroArch, though it can easily be used for other things.

RetroArch for the Raspberry Pi can be found here: https://github.com/ToadKing/RetroArch-Rpi
I'm not associated with RetroArch in any way!

Building
========

EmulationStation has quite a few dependencies. For building, you'll need SDL 1.2, the SDL TTF library, the SDL image library, the SDL_gfx library, and Boost.Filesystem, which can easily be obtained with apt-get:
```
sudo apt-get install libsdl1.2-dev libsdl-ttf2.0-dev libboost-filesystem-dev libsdl-image1.2-dev libsdl-gfx1.2-dev
```

You can build EmulationStation by simply running `make`.

Configuring
===========

When first run, an example systems configuration file will be created at $HOME/.emulationstation/es_systems.cfg. This example has some comments explaining how to write the configuration file, and an example RetroArch launch command. Keep in mind you can define more than one system! Just use all the variables again.

If an SDL Joystick is detected at startup, and $HOME/.emulationstation/es_input.cfg is nonexistant, an Input Configuration screen will appear instead of the game list. This should be pretty self-explanatory. If you want to reconfigure, just delete $HOME/.emulationstation/es_input.cfg.

Mappings will always be applied to the SDL joystick at index 0. An Xbox 360 controller with the xboxdrv driver was tested. POV hats are automatically mapped to directions (so if you're not using an analog stick, you'll need to skip mapping Up/Down/Left/Right by pressing a keyboard key).

Keep in mind you'll have to set up your emulator separately from EmulationStation. If you're using RetroArch, a handy input config generation tool can be found in the tools/ subdirectory - you can use it with `retroarch-joyconfig -o ~/.retroarch.cfg` or something similar. You may need to tell RetroArch to load this config file with `-c ~/.retroarch.cfg` in your RetroArch launch commands.

EmulationStation will return once your system's command terminates (i.e. your emulator closes). To close EmulationStation itself, you can press the F4 key on the keyboard. You can also press F1 to open a menu allowing you to shutdown or restart the system.


gamelist.xml
============

If a file named gamelist.xml is found in the root of a system's search directory, it will be parsed and the detailed GuiGameList will be used. This means you can define images, descriptions, and different names for files.
Images are meant to be 256x256, but ES won't stop you from using other sizes - they'll just be placed wrong. I'd like to add automatic scaling (with SDL_gfx) in a future update.
An example gamelist.xml:
```
<gameList>
	<game>
		<path>/home/pi/ROMs/nes/mm2.nes</path>
		<name>Mega Man 2</name>
		<desc>Mega Man 2 is a classic NES game which follows Mega Man as he murders eight robot masters.</desc>
		<image>/home/pi/Screenshots/megaman2.png</image>
	</game>
</gameList>
```

The path element should be the absolute path of the ROM. Special characters SHOULD NOT be escaped. The image element is the absolute path to an image to display above the description (like a screenshot or boxart). Most formats can be used (including png, jpg, gif, etc.). Not all elements need to be used.

The switch `--gamelist-only` can be used to skip automatic searching, and only display games defined in the system's gamelist.xml.


Themes
======

At the moment, theming is still in flux. But if you want to play around with what's here, you can place a theme.xml file in a system's directory. It will be automatically loaded if present (and you're using the detailed view - any system has a gamelist.xml set up).
Themes are drawn before the rest of the game list. Here's the example I've been using to test a background:

```
<theme>
        <component>
                <type>image</type>
                <path>/home/aloshi/EmulationStation/theme/background.png</path>
                <pos>0 0</pos>
                <dim>1 1</dim>
        </component>
</theme>
```

You can add more than one component. You can use more than one component and components can be nested for your own personal use (but they won't inherit positions or anything). The only type thus far is image. Pos is short for position and dim is short for dimensions. Both work in screen percentages - a decimal from 0 to 1. A single space separates X/Y or width/height.
At the moment the X position is the horizontal center point for the image and Y is the top of the image.
Variable support is present, but the only variable defined right now is $headerHeight. You should be able to use addition/subtraction/multiplication/division.

-Aloshi
http://www.aloshi.com

