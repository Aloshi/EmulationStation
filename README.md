EmulationStation
================

This is a fork of EmulationStation for RetroPie.
EmulationStation is a cross-platform graphical front-end for emulators with controller navigation.

Building
========

EmulationStation uses some C++11 code, which means you'll need to use at least g++-4.7 on Linux, or VS2010 on Windows, to compile.

EmulationStation has a few dependencies. For building, you'll need CMake, SDL2, FreeImage, FreeType, and cURL.  You also should probably install the `fonts-droid` package which contains fallback fonts for Chinese/Japanese/Korean characters, but ES will still work fine without it (this package is only used at run-time).

**On Debian/Ubuntu:**
All of this be easily installed with apt-get:
```bash
sudo apt-get install libsdl2-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev \
  libasound2-dev libgl1-mesa-dev build-essential cmake fonts-droid-fallback libvlc-dev \
  libvlccore-dev vlc-bin
```
**On Fedora:**
All of this be easily installed with dnf ( With rpmfusion activated) :
```bash
sudo dnf install SDL2-devel freeimage-devel freetype-devel curl-devel \
  alsa-lib-devel mesa-libGL-devel cmake \
  vlc-devel
```

Note this Repository uses a git submodule - to checkout the source and all submodules, use

```bash
git clone --recursive https://github.com/RetroPie/EmulationStation.git
```

or 

```bash
git clone https://github.com/RetroPie/EmulationStation.git
cd EmulationStation
git submodule update --init
```

Then, generate and build the Makefile with CMake:
```bash
cd YourEmulationStationDirectory
cmake .
make
```

**On the Raspberry Pi:**

Complete Raspberry Pi build instructions at [emulationstation.org](http://emulationstation.org/gettingstarted.html#install_rpi_standalone).

**On Windows:**

[FreeImage](http://downloads.sourceforge.net/freeimage/FreeImage3154Win32.zip)

[FreeType2](http://download.savannah.gnu.org/releases/freetype/freetype-2.4.9.tar.bz2) (you'll need to compile)

[SDL2](http://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip)

[cURL](http://curl.haxx.se/download.html) (you'll need to compile or get the pre-compiled DLL version)

(Remember to copy necessary .DLLs into the same folder as the executable: probably FreeImage.dll, freetype6.dll, SDL2.dll, libcurl.dll, and zlib1.dll. Exact list depends on if you built your libraries in "static" mode or not.)

[CMake](http://www.cmake.org/cmake/resources/software.html) (this is used for generating the Visual Studio project)

(If you don't know how to use CMake, here are some hints: run cmake-gui and point it at your EmulationStation folder.  Point the "build" directory somewhere - I use EmulationStation/build.  Click configure, choose "Visual Studio [year] Project", fill in red fields as they appear and keep clicking Configure (you may need to check "Advanced"), then click Generate.)


Configuring
===========

**~/.emulationstation/es_systems.cfg:**
When first run, an example systems configuration file will be created at `~/.emulationstation/es_systems.cfg`.  `~` is `$HOME` on Linux, and `%HOMEPATH%` on Windows.  This example has some comments explaining how to write the configuration file. See the "Writing an es_systems.cfg" section for more information.

**Keep in mind you'll have to set up your emulator separately from EmulationStation!**

**~/.emulationstation/es_input.cfg:**
When you first start EmulationStation, you will be prompted to configure an input device. The process is thus:

1. Hold a button on the device you want to configure.  This includes the keyboard.

2. Press the buttons as they appear in the list.  Some inputs can be skipped by holding any button down for a few seconds (e.g. page up/page down).

3. You can review your mappings by pressing up and down, making any changes by pressing A.

4. Choose "SAVE" to save this device and close the input configuration screen.

The new configuration will be added to the `~/.emulationstation/es_input.cfg` file.

**Both new and old devices can be (re)configured at any time by pressing the Start button and choosing "CONFIGURE INPUT".**  From here, you may unplug the device you used to open the menu and plug in a new one, if necessary.  New devices will be appended to the existing input configuration file, so your old devices will remain configured.

**If your controller stops working, you can delete the `~/.emulationstation/es_input.cfg` file to make the input configuration screen re-appear on next run.**


You can use `--help` or `-h` to view a list of command-line options. Briefly outlined here:
```
--resolution [width] [height]	- try and force a particular resolution
--gamelist-only		- only display games defined in a gamelist.xml file.
--ignore-gamelist	- do not parse any gamelist.xml files.
--draw-framerate	- draw the framerate.
--no-exit		- do not display 'exit' in the ES menu.
--debug			- show the console window on Windows, do slightly more logging
--windowed		- run ES in a window, works best in conjunction with --resolution [w] [h].
--vsync [1/on or 0/off]	- turn vsync on or off (default is on).
--scrape		- run the interactive command-line metadata scraper.
--no-splash		- don't show the splash screen.
--max-vram [size]	- Max VRAM to use in Mb before swapping. 0 for unlimited.
--force-kiosk		- Force the UI mode to be Kiosk.
```

As long as ES hasn't frozen, you can always press F4 to close the application.


Writing an es_systems.cfg
=========================

Complete configuration instructions at [emulationstation.org](http://emulationstation.org/gettingstarted.html#config).

The `es_systems.cfg` file contains the system configuration data for EmulationStation, written in XML.  This tells EmulationStation what systems you have, what platform they correspond to (for scraping), and where the games are located.

ES will check two places for an es_systems.cfg file, in the following order, stopping after it finds one that works:
* `~/.emulationstation/es_systems.cfg`
* `/etc/emulationstation/es_systems.cfg`

The order EmulationStation displays systems reflects the order you define them in.

**NOTE:** A system *must* have at least one game present in its "path" directory, or ES will ignore it! If no valid systems are found, ES will report an error and quit!

Here's an example es_systems.cfg:

```xml
<!-- This is the EmulationStation Systems configuration file.
All systems must be contained within the <systemList> tag.-->

<systemList>
	<!-- Here's an example system to get you started. -->
	<system>
		<!-- A short name, used internally. -->
		<name>snes</name>

		<!-- A "pretty" name, displayed in the menus and such. This one is optional. -->
		<fullname>Super Nintendo Entertainment System</fullname>

		<!-- The path to start searching for ROMs in. '~' will be expanded to $HOME or %HOMEPATH%, depending on platform.
		All subdirectories (and non-recursive links) will be included. -->
		<path>~/roms/snes</path>

		<!-- A list of extensions to search for, delimited by any of the whitespace characters (", \r\n\t").
		You MUST include the period at the start of the extension! It's also case sensitive. -->
		<extension>.smc .sfc .SMC .SFC</extension>

		<!-- The shell command executed when a game is selected. A few special tags are replaced if found in a command, like %ROM% (see below). -->
		<command>snesemulator %ROM%</command>
		<!-- This example would run the bash command "snesemulator /home/user/roms/snes/Super\ Mario\ World.sfc". -->

		<!-- The platform(s) to use when scraping. You can see the full list of accepted platforms in src/PlatformIds.cpp.
		It's case sensitive, but everything is lowercase. This tag is optional.
		You can use multiple platforms too, delimited with any of the whitespace characters (", \r\n\t"), eg: "genesis, megadrive" -->
		<platform>snes</platform>

		<!-- The theme to load from the current theme set. See THEMES.md for more information.
		This tag is optional; if not set, it will use the value of <name>. -->
		<theme>snes</theme>
	</system>
</systemList>
```

The following "tags" are replaced by ES in launch commands:

`%ROM%`		- Replaced with absolute path to the selected ROM, with most Bash special characters escaped with a backslash.

`%BASENAME%`	- Replaced with the "base" name of the path to the selected ROM. For example, a path of "/foo/bar.rom", this tag would be "bar". This tag is useful for setting up AdvanceMAME.

`%ROM_RAW%`	- Replaced with the unescaped, absolute path to the selected ROM.  If your emulator is picky about paths, you might want to use this instead of %ROM%, but enclosed in quotes.

See [SYSTEMS.md](SYSTEMS.md) for some live examples in EmulationStation.

gamelist.xml
============

The gamelist.xml file for a system defines metadata for games, such as a name, image (like a screenshot or box art), description, release date, and rating.

If at least one game in a system has an image specified, ES will use the detailed view for that system (which displays metadata alongside the game list).

*You can use ES's [scraping](http://en.wikipedia.org/wiki/Web_scraping) tools to avoid creating a gamelist.xml by hand.*  There are two ways to run the scraper:

* **If you want to scrape multiple games:** press start to open the menu and choose the "SCRAPER" option.  Adjust your settings and press "SCRAPE NOW".
* **If you just want to scrape one game:** find the game on the game list in ES and press select.  Choose "EDIT THIS GAME'S METADATA" and then press the "SCRAPE" button at the bottom of the metadata editor.

You can also edit metadata within ES by using the metadata editor - just find the game you wish to edit on the gamelist, press Select, and choose "EDIT THIS GAME'S METADATA."

A command-line version of the scraper is also provided - just run emulationstation with `--scrape` *(currently broken)*.

The switch `--ignore-gamelist` can be used to ignore the gamelist and force ES to use the non-detailed view.

If you're writing a tool to generate or parse gamelist.xml files, you should check out [GAMELISTS.md](GAMELISTS.md) for more detailed documentation.


Themes
======

By default, EmulationStation looks pretty ugly. You can fix that. If you want to know more about making your own themes (or editing existing ones), read [THEMES.md](THEMES.md)!

I've put some themes up for download on my EmulationStation webpage: http://aloshi.com/emulationstation#themes

If you're using RetroPie, you should already have a nice set of themes automatically installed!


-Alec "Aloshi" Lofquist
http://www.aloshi.com
http://www.emulationstation.org
