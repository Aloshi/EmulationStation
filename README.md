EmulationStation
================

A simple front-end for emulators made with SDL, designed for controller navigation. Developed for use with the Raspberry Pi and RetroArch, though it can easily be used for other things.

RetroArch for the Raspberry Pi can be found here: https://github.com/ToadKing/RetroArch-Rpi
I'm not associated with RetroArch in any way!

Building
========

EmulationStation has a few dependencies. For building, you'll need SDL 1.2, the SDL TTF library, the SDL image library, and Boost.Filesystem, which can easily be obtained with apt-get:
```sudo apt-get install libsdl1.2-dev
sudo apt-get install libsdl-ttf2.0-dev
sudo apt-get install libboost-filesystem-dev
sudo apt-get install libsdl-image1.2-dev
```

You can build EmulationStation by simply running `make`.

Configuring
===========

When first run, an example systems configuration file will be created at $HOME/.es_systems.cfg. This example has some comments explaining how to write the configuration file, and an example RetroArch launch command. Keep in mind you can define more than one system! Just use all the variables again.

If an SDL Joystick is detected at startup, and $HOME/.es_input.cfg is nonexistant, an Input Configuration screen will appear instead of the game list. This should be pretty self-explanatory. If you want to reconfigure, just delete $HOME/.es_input.cfg.

Mappings will always be applied to the SDL joystick at index 0. An Xbox 360 controller with the xboxdrv driver was tested. POV hats are automatically mapped to directions (so if you're not using an analog stick, you'll need to skip mapping Up/Down/Left/Right by pressing a keyboard key).

Keep in mind you'll have to set up your emulator separately from EmulationStation. If you're using RetroArch, a handy input config generation tool can be found in the tools/ subdirectory - you can use it with `retroarch-joyconfig -o ~/.retroarch.cfg` or something similar. You may need to tell RetroArch to load this config file with `-c ~/.retroarch.cfg` in your RetroArch launch commands.

EmulationStation will return once your system's command terminates (i.e. your emulator closes). To close EmulationStation itself, you can press the F4 key on the keyboard.


gamelist.xml
============

If a file named gamelist.xml is found, it will be parsed and the detailed GuiGameList will be used. This means you can define screenshots, descriptions, and alternate names for files.
Screenshots are meant to be 256x256, but ES won't stop you from using other sizes - they'll just be placed wrong.
An example gamelist.xml:
```<gameList>
	<game>
		<path>/home/pi/ROMs/nes/mm2.nes</path>
		<name>Mega Man 2</name>
		<desc>Mega Man 2 is a classic NES game which follows Mega Man as he murders eight robot masters.</desc>
		<image>/home/pi/Screenshots/megaman2.png</image>
	</game>
</gameList>
```

The path element should be the absolute path of the ROM. Special characters SHOULD NOT be escaped. The image element is the absolute path to an image to use (like a screenshot or boxart). Most formats can be used (including png, jpg, gif, etc.). Look up the SDL_image library for a full list. Not all elements need to be used.


-Aloshi
http://www.aloshi.com
