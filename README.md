EmulationStation
================

A cross-platform graphical front-end for emulators with controller navigation.

**Raspberry Pi users:**
A cool guy named petrockblog made a script which automatically installs many emulators and ES. It also includes options for configuring your RPi and setting it up to boot directly into ES. You can find it here: https://github.com/petrockblog/RetroPie-Setup

I found a bug! I have a problem!
================================

- First, try to check the [issue list](https://github.com/Aloshi/EmulationStation/issues?state=open) for some entries that might match your problem.  Make sure to check closed issues too!
- If you're running EmulationStation on a on Raspberry Pi and have problems with config file changes not taking effect, content missing after editing, etc., check if your SD card is corrupted (see issues [#78](https://github.com/Aloshi/EmulationStation/issues/78) and [#107](https://github.com/Aloshi/EmulationStation/issues/107)). You can do this with free tools like [h2testw](http://www.heise.de/download/h2testw.html) or [F3](http://oss.digirati.com.br/f3/).
- Try to update to the latest version of EmulationStation using git (you might need to delete your `es_input.cfg` and `es_settings.cfg` after that to reset them to default values):
```bash
cd EmulationStation
git pull
export CXX=g++-4.7
cmake .
make
```
- If your problem still isn't gone, the best way to report a bug is to post an issue on GitHub. Try to post the simplest steps possible to reproduce the bug. Include files you think might be related (except for ROMs, of course). If you haven't re-run ES since the crash, the log file `~/.emulationstation/es_log.txt` is also helpful.

Building
========

EmulationStation uses some C++11 code, which means you'll need to install at least g++-4.7 on Linux, or VS2010 on Windows. 
For installing and switching to g++-4.7 see [here](http://lektiondestages.blogspot.de/2013/05/installing-and-switching-gccg-versions.html).  You can also just use `export CXX=g++-4.7` to explicitly specify the compiler for CMake (make sure you delete your CMake cache files if it's not working).

EmulationStation has a few dependencies. For building, you'll need SDL2, Boost (System, Filesystem, DateTime), FreeImage, FreeType, Eigen3, and cURL.

**On Linux:**
All of this be easily installed with apt-get:
```bash
sudo apt-get install libsdl2-dev libboost-dev libboost-system-dev libboost-filesystem-dev libboost-date-time-dev libfreeimage-dev libfreetype6-dev libeigen3-dev libcurl-dev libasound2-dev
```

Unless you're on the Raspberry Pi, you'll also need OpenGL:
```bash
sudo apt-get install libgl1-mesa-dev
```

On the Raspberry Pi, there are also a few special libraries, located in /opt/vc/: the Broadcom libraries, libEGL, and GLES.  You shouldn't need to install them; they are used by the Raspberry Pi port of SDL 2.

**Generate and Build Makefile with CMake:**
```
cd EmulationStation
export CXX=g++-4.7
cmake .
make
```

**On Windows:**

[Boost](http://www.boost.org/users/download/) (you'll need to compile yourself or get the pre-compiled binaries)

[Eigen3](http://eigen.tuxfamily.org/index.php?title=Main_Page)

[FreeImage](http://downloads.sourceforge.net/freeimage/FreeImage3154Win32.zip)

[FreeType2](http://download.savannah.gnu.org/releases/freetype/freetype-2.4.9.tar.bz2) (you'll need to compile)

[SDL2](http://www.libsdl.org/release/SDL2-devel-2.0.0-VC.zip)

[CURL](http://curl.haxx.se/download.html) (you'll need to compile or get the pre-compiled (DLL version))

(remember to copy necessary .DLLs into the same folder as the executable: FreeImage.dll, freetype6.dll, SDL2.dll, and zlib1.dll)

[CMake](http://www.cmake.org/cmake/resources/software.html) (this is used for generating the Visual Studio project)

(If you don't know how to use CMake, here are some hints: run cmake-gui and point it at your EmulationStation folder.  Point the "build" directory somewhere - I use EmulationStation/build.  Click configure, choose "Visual Studio [year] Project", fill in red fields as they appear and keep clicking Configure, then click Generate.)


Configuring
===========

**~/.emulationstation/es_systems.cfg:**
When first run, an example systems configuration file will be created at $HOME/.emulationstation/es_systems.cfg. This example has some comments explaining how to write the configuration file, and an example RetroArch launch command. See the "Writing an es_systems.cfg" section for more information.

**Keep in mind you'll have to set up your emulator separately from EmulationStation!**

After you launch a game, EmulationStation will return once your system's command terminates (i.e. your emulator closes).

**~/.emulationstation/es_input.cfg:**
When you first start EmulationStation, you will be prompted to configure an input device. The process is thus:

1. Hold a button on the device you want to configure.  This includes the keyboard.

2. Press the buttons as they appear in the list.  Some inputs can be skipped by holding any button down for a few seconds (e.g. page up/page down).

3. You can review your mappings by pressing up and down, making any changes by pressing A.

4. Choose "SAVE" to save this device and close the input configuration screen.

The new configuration will be added to the `~/.emulationstation/es_input.cfg` file.

**Both new and old devices can be (re)configured at any time by pressing the Start button and choosing "CONFIGURE INPUT".**  From here, you may unplug the device you used to open the menu and plug in a new one, if necessary.  New devices will be appended to the existing input configuration file, so your old devices will remain configured.

**If things stop working, you can delete the `~/.emulationstation/es_input.cfg` file to make the input configuration screen reappear on next run.**


You can use `--help` or `-h` to view a list of command-line options. Briefly outlined here:
```
--resolution [width] [height]	- try and force a particular resolution
--gamelist-only		- only display games defined in a gamelist.xml file.
--ignore-gamelist	- do not parse any gamelist.xml files.
--draw-framerate	- draw the framerate.
--no-exit		- do not display 'exit' in the ES menu.
--debug			- print additional output to the console, primarily about input.
--windowed	- run ES in a window, works best in conjunction with --resolution [w] [h].
--scrape	- run the interactive command-line metadata scraper.
--home-path [path]	- use [path] instead of the "home" environment variable (useful for portable installations).
```

As long as ES hasn't frozen, you can always press F4 to close the application.


Writing an es_systems.cfg
=========================

The `es_systems.cfg` file contains the system configuration data for EmulationStation, written in XML.  This tells EmulationStation what systems you have, what platform they correspond to (for scraping), and where the games are located.

ES will check two places for an es_systems.cfg file, in the following order:
* `~/.emulationstation/es_systems.cfg`
* `/etc/emulationstation/es_systems.cfg`

The order EmulationStation displays systems reflects the order you define them in.

**NOTE:** A system *must* have at least one game present in its "path" directory, or ES will ignore it! If no systems are found, ES won't even start!

Here's an example es_systems.cfg:

```xml
<!-- This is the EmulationStation Systems configuration file.
All systems must be contained within the <systemList> tag.-->

<systemList>
	<!-- Here's an example system to get you started. -->
	<system>
		<!-- A short name, used internally. -->
		<name>SNES</name>

		<!-- A "pretty" name, displayed in the menus and such. This one is optional. -->
		<fullname>Super Nintendo Entertainment System</fullname>

		<!-- The path to start searching for ROMs in. '~' will be expanded to $HOME or $HOMEPATH, depending on platform. 
		All subdirectories (and non-recursive links) will be included. -->
		<path>~/roms/snes</path>

		<!-- A list of extensions to search for, delimited by a space. You MUST include the period! It's also case sensitive. -->
		<extension>.smc .sfc .SMC .SFC</extension>

		<!-- The shell command executed when a game is selected. A few special tags are replaced if found in a command, like %ROM%. -->
		<command>snesemulator %ROM%</command>
		<!-- This example would run the bash command "snesemulator /home/user/roms/snes/Super\ Mario\ World.sfc". -->

		<!-- The platform to use when scraping. You can see the full list of accepted platforms in src/PlatformIds.cpp. -->
		<platform>snes</platform>
	</system>
</systemList>
```

The following "tags" are replaced by ES in launch commands:

`%ROM%`		- Replaced with absolute path to the selected ROM, with most Bash special characters escaped with a backslash.

`%BASENAME%`	- Replaced with the "base" name of the path to the selected ROM. For example, a path of "/foo/bar.rom", this tag would be "bar". This tag is useful for setting up AdvanceMAME.

`%ROM_RAW%`	- Replaced with the unescaped absolute path to the selected ROM.  If your emulator is picky about paths, you might want to use this instead of %ROM%, but enclosed in quotes.


gamelist.xml
============

The gamelist.xml for a system defines metadata for a system's games, such as a name, image (like a screenshot or box art), description, release date, and rating.

ES will check three places for a gamelist.xml, in the following order:
* `[SYSTEM_PATH]/gamelist.xml`
* `~/.emulationstation/gamelists/[SYSTEM_NAME]/gamelist.xml`
* `/etc/emulationstation/gamelists/[SYSTEM_NAME]/gamelist.xml`

This file allows you to define images, descriptions, and different names for files. Note that only standard ASCII characters are supported for text (if you see a weird [X] symbol, you're probably using Unicode!).
Images will be automatically resized to fit within the left column of the screen. Smaller images will load faster, so try to keep your resolution low.
An example gamelist.xml:
```xml
<gameList>
	<game>
		<path>/home/pi/ROMs/nes/mm2.nes</path>
		<name>Mega Man 2</name>
		<desc>Mega Man 2 is a classic NES game which follows Mega Man as he murders eight robot masters.</desc>
		<image>/home/pi/Screenshots/megaman2.png</image>
	</game>
</gameList>
```

The path element should be either the absolute path of the ROM, or a path relative to the system games folder that starts with "./".  For example:

`<path>/home/pi/ROMs/nes/mm2.nes</path>`

or

`<path>./mm2.nes</path>`

ES will attempt to encode paths as relative to the system's `path` setting whenever it writes a gamelist.xml. Special characters SHOULD NOT be escaped. The image element is the path to an image to display above the description (like a screenshot or boxart). Most popular image formats can be used (png, jpg, gif, etc.). Not all elements need to be used.

The switch `--gamelist-only` can be used to skip automatic searching, and only display games defined in the system's gamelist.xml.
The switch `--ignore-gamelist` can be used to ignore the gamelist and force ES to use the non-detailed view.

*You can use ES's [scraping](http://en.wikipedia.org/wiki/Web_scraping) tools to avoid creating a gamelist.xml by hand.*  There are two ways to run the scraper:

* **If you want to scrape multiple games:** press start to open the menu and choose the "SCRAPER" option.  Adjust your settings and press "SCRAPE NOW".
* **If you just want to scrape one game:** find the game on the game list in ES and press select.  Choose "EDIT THIS GAME'S METADATA" and then press the "SCRAPE" button at the bottom of the metadata editor.

You can also edit metadata within ES by using the metadata editor - just find the game you wish to edit on the gamelist, press select, and choose "EDIT THIS GAME'S METADATA."

If you're writing a tool to generate gamelist.xml files, you can see the complete list of possible elements at the top of `src/MetaData.cpp`.

A command-line version of the scraper is also provided - just run emulationstation with `--scrape` *(currently broken)*.

Themes
======

By default, EmulationStation looks pretty ugly. You can fix that. If you want to know more about making your own themes (or editing existing ones), read THEMES.md!

I've put some themes up for download on my EmulationStation webpage: http://aloshi.com/emulationstation#themes

If you're using RetroPie, you should already have a nice set of themes automatically installed!

-Aloshi
http://www.aloshi.com
