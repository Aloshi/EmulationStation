EmulationStation
================

A graphical front-end for emulators with controller navigation. Developed for the Raspbery Pi, but runs on most Linux systems.

A cool guy named petrockblog made a script which automatically installs RetroArch, its cores, and ES. It also includes options for configuring your RPi and setting it up to boot directly into ES. You can find it here: https://github.com/petrockblog/RetroPie-Setup

Building
========

**On the Raspberry Pi:**

EmulationStation has a few dependencies. For building, you'll need SDL 1.2, SDL_mixer, FreeImage, FreeType, and Boost.Filesystem, which can easily be obtained with apt-get:
```
sudo apt-get install libsdl1.2-dev libboost-system-dev libboost-filesystem-dev libfreeimage-dev libfreetype6-dev ttf-dejavu
```

There are also a few libraries already on the RPi (located in /opt/vc/, like the Broadcom libraries, EGL, and GLES). You can build EmulationStation by simply running `make`.


**On something else (desktop):**

EmulationStation can also be built on a "normal" Linux system. You'll need the same libraries listed above:
```
sudo apt-get install libsdl1.2-dev libboost-system-dev libboost-filesystem-dev libfreeimage-dev libfreetype6-dev ttf-dejavu
```

You'll also need OpenGL. I you don't have `/usr/include/GL/gl.h` and `libGL` try installing the MESA development package with:
```
sudo apt-get libgl1-mesa-dev
```

You can build with `make -f Makefile.x86` (badly named Makefile, I know). For some reason it doesn't seem to run with X closed on desktop.


**Via CMake:**
<pre>
cd EmulationStation
cmake .
make
</pre>

Configuring
===========

**~/.emulationstation/es_systems.cfg:**
When first run, an example systems configuration file will be created at $HOME/.emulationstation/es_systems.cfg. This example has some comments explaining how to write the configuration file, and an example RetroArch launch command. See the "Writing an es_systems.cfg" section for more information.

**~/.emulationstation/es_input.cfg:**
When you first start EmulationStation, you will be prompted to configure any input devices you wish to use. The process is thus:

1. Press a button on any device you wish to use. *This includes the keyboard.* If you are unable to configure a device, hold a button on the first device to continue to step 2.

2. Press the displayed input for each device in sequence.  You will be prompted for Up, Down, Left, Right, A (Select), B (Back), Menu, Select (fast select), PageUp, and PageDown, Volume up and Volume down. If your controller doesn't have enough buttons to map PageUp/PageDown, it will be skipped.

3. Your config will be saved to `~/.emulationstation/es_input.cfg`. If you wish to reconfigure, just delete this file.

*NOTE: If `~/.emulationstation/es_input.cfg` is present but does not contain any available joysticks or a keyboard, an emergency default keyboard mapping will be provided.*

As long as ES hasn't frozen, you can always press F4 to close the application.


**Keep in mind you'll have to set up your emulator separately from EmulationStation.**
I am currently also working on a stand-alone tool, [ES-config](https://github.com/Aloshi/ES-config), that will help make configuring emulators easier.

After you launch a game, EmulationStation will return once your system's command terminates (i.e. your emulator closes).


You can use `--help` to view a list of command-line options. Briefly outlined here:
```
-w [width]		- specify resolution width.
-h [height]		- specify resolution height.
--gamelist-only		- only display games defined in a gamelist.xml file.
--ignore-gamelist	- do not parse any gamelist.xml files.
--draw-framerate	- draw the framerate.
--no-exit		- do not display 'exit' in the ES menu.
--debug			- print additional output to the console, primarily about input.
--dimtime [seconds]	- delay before dimming the screen and entering sleep mode. Default is 30, use 0 for never.
--windowed      - run ES in a window.
```

Writing an es_systems.cfg
=========================
The file `~/.emulationstation/es_systems.cfg` contains the system configuration data for EmulationStation. A system is a NAME, DESCNAME, PATH, EXTENSION, and COMMAND. You can define any number of systems, just use every required variable again. You can switch between systems by pressing left and right. They will cycle in the order they are defined.

The NAME is what ES will use to internally identify the system. Theme.xml and gamelist.xml files will also be searched for in `~/.emulationstation/NAME/` if not found at the root of PATH. It is recommended that you abbreviate here if necessary, e.g. "nes".

The DESCNAME is a "pretty" name for the system - it show up in a header if one is displayed. It is optional; if not supplied, it will copy NAME (note: DESCNAME must also *not* be the last tag you define for a system! This is due to the nature of how optional tags are implemented.).

The PATH is where ES will start the search for ROMs. All subdirectories (and links!) will be included.

**NOTE:** A system *must* have at least one game present in its PATH directory, or ES will ignore it.

The EXTENSION is a list of extensions ES will consider valid and add to the list when searching. Each extension *must* start with a period. The list is delimited by a space.

The COMMAND is the shell command ES will execute to start your emulator. As it is evaluated by the shell (i.e. bash), you can do some clever tricks if need be.

The following "tags" are replaced by ES in COMMANDs:

`%ROM%`		- Replaced with absolute path to the selected ROM.

`%BASENAME%`	- Replaced with the "base" name of the path to the selected ROM. For example, a path of "/foo/bar.rom", this tag would be "bar". This tag is useful for setting up AdvanceMAME.

gamelist.xml
============

The gamelist.xml for a system defines metadata for a system's games. This metadata includes an image (e.g. screenshot or box art), description, and name.

**Making a gamelist.xml by hand sucks, so a cool guy named Pendor made a python script which automatically generates a gamelist.xml for you, with boxart automatically downloaded. It can be found here:** https://github.com/elpendor/ES-scraper

If a file named gamelist.xml is found in the root of a system's search directory OR within `~/.emulationstation/%NAME%/`, it will be parsed and the detailed GuiGameList will be used. This means you can define images, descriptions, and different names for files. Note that only standard ASCII characters are supported (if you see a weird [X] symbol, you're probably using unicode!).
Images will be automatically resized to fit within the left column of the screen. Smaller images will load faster, so try to keep your resolution low.
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

The path element should be the absolute path of the ROM. Special characters SHOULD NOT be escaped. The image element is the path to an image to display above the description (like a screenshot or boxart). Most formats can be used (including png, jpg, gif, etc.). Not all elements need to be used.

The switch `--gamelist-only` can be used to skip automatic searching, and only display games defined in the system's gamelist.xml.
The switch `--ignore-gamelist` can be used to ignore the gamelist and use the non-detailed view.

I found a bug!
==============

The best way to report a bug is to post an issue on GitHub. Try to post the simplest steps possible to reproduce the bug. Include files you think might be related (except for ROMs, of course). If you haven't re-run ES since the crash, the log file `~/.emulationstation/es_log.txt` is also helpful.

Themes
======

By default, EmulationStation looks pretty ugly. You can fix that. If you want to know more about making your own themes (or editing existing ones), read THEMES.md!

I've put some themes up for download on my EmulationStation webpage: http://aloshi.com/emulationstation#themes

-Aloshi
http://www.aloshi.com
