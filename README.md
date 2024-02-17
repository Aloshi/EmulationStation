EmulationStation
================

This is a fork of EmulationStation for RetroPie.
EmulationStation is a cross-platform graphical front-end for emulators with controller navigation.

Building
========

Building on Linux
-----------------

EmulationStation uses some C++11 code, which means you'll need to use at least g++-4.7 on Linux, or VS2010 on Windows, to compile.

EmulationStation has a few dependencies. For building, you'll need CMake, SDL2, FreeImage, FreeType, LibVLC (ver. 3 or later), cURL and RapidJSON.  You also should probably install the `fonts-droid` package which contains fallback fonts for Chinese/Japanese/Korean characters, but ES will still work fine without it (this package is only used at run-time).

### On Debian/Ubuntu:
All of this be easily installed with `apt-get`:
```bash
sudo apt-get install libsdl2-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev rapidjson-dev \
  libasound2-dev libgles2-mesa-dev build-essential cmake fonts-droid-fallback libvlc-dev \
  libvlccore-dev vlc-bin
```
### On Fedora:
All of this be easily installed with `dnf` (with rpmfusion activated) :
```bash
sudo dnf install SDL2-devel freeimage-devel freetype-devel curl-devel \
  alsa-lib-devel mesa-libGL-devel cmake \
  vlc-devel rapidjson-devel
```

 Optionaly, `pugixml` can be installed and used (Debian package: `libpugixml-dev`, Fedora/SuSE package: `pugixml-devel`), but EmulationStation can use its own included copy if not found.
 
**Note**: this repository uses a git submodule - to checkout the source and all submodules, use

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

NOTE: to generate a `Debug` build on Unix/Linux, run the Makefile generation step as:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug .
```

### On the Raspberry Pi:

* Choosing a GLES implementation.

   * if the Pi system uses the legacy/Broadcom driver, install the `libraspberry-dev` package before running `cmake` to configure the build
   * if the Pi system uses the Mesa VC4/V3D GL driver, build using `-DUSE_MESA_GLES=On` to choose the MESA GLES implementation. This option is _mandatory_ when compiling for a Pi4 system, since the legacy GL drivers are not supported anymore on this system.

  NOTE: Starting with RasPI OS 'Bullseye', the legacy/Broadcom drivers are not supported anymore, so `-DUSE_MESA_GLES=On` should be used.

* Enable the audio/memory defaults by adding `-DRPI=On` to the build options
* Support for using `omxplayer` to play video previews in the gamelist is enabled by adding `-DOMX=On` to the build options.
  NOTE: `omxplayer` support is not available on 64bit RasPI OS or in the default RasPI OS 'Bullseye' configuration.

**GLES build notes**

 If your system doesn't have a working GLESv2 implementation, the GLESv1 legacy renderer can be compiled in by adding `-DUSE_GLES1=On` to the build options.

Building on Windows
-------------------

* Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/). At a minimum, install the "Desktop development with C++" workload with the default list of optional items.

* Install the latest version of [CMake](https://cmake.org/download/) (e.g., [cmake-3.27.1-windows-x86_64.msi](https://github.com/Kitware/CMake/releases/download/v3.27.1/cmake-3.27.1-windows-x86_64.msi)). CMake is used for generating the Visual Studio project.

* Use git to clone [vcpkg](https://vcpkg.io/en/), then run the bootstrap script as shown below to build vcpkg . This is a C/C++ dependency manager from Microsoft.

```batchfile
C:\src>git clone https://github.com/Microsoft/vcpkg.git
C:\src>.\vcpkg\bootstrap-vcpkg.bat
```

* Download the latest [nuget.exe](https://dist.nuget.org/win-x86-commandline/latest/nuget.exe) to a folder that is in your Windows PATH. NuGet is a .NET package manager.

* Use NuGet to download the latest [libVLC](https://www.videolan.org/vlc/libvlc.html). This library is used to play video snaps.

```batchfile
C:\src\EmulationStation>mkdir nuget
C:\src\EmulationStation>cd nuget
C:\src\EmulationStation\nuget>nuget install -ExcludeVersion VideoLAN.LibVLC.Windows
```

* Use vcpkg to download the latest pre-compiled [cURL](http://curl.haxx.se/download.html). This is a library for transferring data with URLs.

```batchfile
c:\src>.\vcpkg\vcpkg install curl:x86-windows-static-md
```

* Use vcpkg to download the latest [FreeImage](https://freeimage.sourceforge.io/index.html). This library supports popular graphics image formats.

```batchfile
c:\src\>.\vcpkg\vcpkg install freeimage:x86-windows-static-md
```

* Use vcpkg to download the latest pre-compiled [FreeType2](https://freetype.org/). This library is used to render fonts.

```batchfile
c:\src>.\vcpkg\vcpkg install freetype:x86-windows-static-md
```

* Use vcpkg to download the latest [SDL2](http://www.libsdl.org/). Simple DirectMedia Layer is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D.

```batchfile
c:\src>.\vcpkg\vcpkg install sdl2:x86-windows-static-md
```

* Use vcpkg to download the latest [RapidJSON](http://rapidjson.org/). This library provides a fast JSON parser/generator for C++ with both SAX/DOM style API.

```batchfile
c:\src>.\vcpkg\vcpkg install rapidjson:x86-windows-static-md
```

* Using the example shown below, configure environment variables to point to the libraries that were installed in the above steps. Please note that the below example intentionally uses forward slashes for compatibility with CMake.

```batchfile
C:\src\EmulationStation>set VCPKG=C:/src/vcpkg/installed/x86-windows-static-md
C:\src\EmulationStation>set NUGET=C:/src/EmulationStation/nuget
C:\src\EmulationStation>set FREETYPE_DIR=%VCPKG%
C:\src\EmulationStation>set FREEIMAGE_HOME=%VCPKG%
C:\src\EmulationStation>set VLC_HOME=%NUGET%/VideoLAN.LibVLC.Windows/build/x86
C:\src\EmulationStation>set RAPIDJSON_INCLUDE_DIRS=%VCPKG%/include
C:\src\EmulationStation>set CURL_INCLUDE_DIR=%VCPKG%/include
C:\src\EmulationStation>set SDL2_INCLUDE_DIR=%VCPKG%/include/SDL2
C:\src\EmulationStation>set VLC_INCLUDE_DIR=%VLC_HOME%/include
C:\src\EmulationStation>set CURL_LIBRARY=%VCPKG%/lib/*.lib
C:\src\EmulationStation>set SDL2_LIBRARY=%VCPKG%/lib/manual-link/SDL2main.lib
C:\src\EmulationStation>set VLC_LIBRARIES=%VLC_HOME%/libvlc*.lib
C:\src\EmulationStation>set VLC_VERSION=3.0.11
```

* Use CMake to generate the Visual Studio project.

```batchfile
C:\src\EmulationStation>mkdir build
C:\src\EmulationStation>cmake . -B build -A Win32 ^
-DRAPIDJSON_INCLUDE_DIRS=%RAPIDJSON_INCLUDE_DIRS% ^
-DCURL_INCLUDE_DIR=%CURL_INCLUDE_DIR% ^
-DSDL2_INCLUDE_DIR=%SDL2_INCLUDE_DIR% ^
-DVLC_INCLUDE_DIR=%VLC_INCLUDE_DIR% ^
-DCURL_LIBRARY=%CURL_LIBRARY% ^
-DSDL2_LIBRARY=%SDL2_LIBRARY% ^
-DVLC_LIBRARIES=%VLC_LIBRARIES% ^
-DVLC_VERSION=%VLC_VERSION% ^
-DCMAKE_EXE_LINKER_FLAGS=/SAFESEH:NO
```

* Use CMake to build the Visual Studio project.

```batchfile
C:\src\EmulationStation>cmake --build build --config Release
```

* Using the example shown below, copy the newly built binaries and other needed files to destination folder.

```batchfile
C:\src\EmulationStation>mkdir -p C:\apps\EmulationStation\.emulationstation
C:\src\EmulationStation>xcopy C:\src\EmulationStation\resources C:\apps\EmulationStation\resources /h /i /c /k /e /r /y
C:\src\EmulationStation>copy C:\src\EmulationStation\Release\*.exe C:\apps\EmulationStation /Y
C:\src\EmulationStation>copy C:\src\EmulationStation\nuget\VideoLAN.LibVLC.Windows\build\x86\*.dll C:\apps\EmulationStation /Y
C:\src\EmulationStation>xcopy C:\src\EmulationStation\nuget\VideoLAN.LibVLC.Windows\build\x86\plugins C:\apps\EmulationStation\plugins /h /i /c /k /e /r /y

```


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

You can use `--help` or `-h` to view a list of command-line options.

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
