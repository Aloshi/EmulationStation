# Systems

This outlines how to add support for many different systems into EmulationStation through configuration of `es_systems.cfg`.

## Nintendo Wii

### [Dolphin](http://dolphin-emu.org/)
In Options → Configure → Interface, disable *Confirm to Stop*.
``` xml
<system>
  <name>wii</name>
  <fullname>Nintendo Wii</fullname>
  <path>/rom/path/here</path>
  <extension>.elf .dol .gcm .iso .wbfs .ciso .gcz .wad</extension>
  <command>dolphin-emu -b -e %ROM%</command>
  <platform>wii</platform>
  <theme>wii</theme>
</system>
```


## [Nintendo GameCube](https://en.wikipedia.org/wiki/GameCube)

### [Dolphin](http://dolphin-emu.org/)
In Options → Configure → Interface, disable *Confirm to Stop*.
``` xml
<system>
  <name>gc</name>
  <fullname>Nintendo GameCube</fullname>
  <path>/rom/path/here</path>
  <extension>.elf .dol .gcm .iso .wbfs .ciso .gcz .wad</extension>
  <command>dolphin-emu -b -e %ROM%</command>
  <platform>gc</platform>
  <theme>gc</theme>
</system>
```


## Nintendo 64

### [RetroArch](http://libretro.com)
Requires a [Nintendo N64 Core](http://wiki.libretro.com/index.php?title=Nintendo_N64_Core_Compatibility), like [`libretro-mupen64plus`](http://wiki.libretro.com/index.php?title=Mupen64Plus).
``` xml
<system>
  <name>n64</name>
  <fullname>Nintendo 64</fullname>
  <path>/path/to/roms</path>
  <extension>.z64 .zip .n64</extension>
  <command>retroarch --fullscreen -L /usr/lib/libretro/mupen64plus_libretro.so %ROM%</command>
  <platform>n64</platform>
  <theme>n64</theme>
</system>
```

### [Mupen64Plus](https://code.google.com/p/mupen64plus/)
``` xml
<system>
  <name>n64</name>
  <fullname>Nintendo 64</fullname>
  <path>/path/to/roms</path>
  <extension>.z64 .zip .n64</extension>
  <command>mupen64plus --nogui --noask --noosd --fullscreen %ROM%</command>
  <platform>n64</platform>
  <theme>n64</theme>
</system>
```


## Nintendo Entertainment System

### [RetroArch](http://libretro.com)
Requires a [Nintendo NES Core](http://wiki.libretro.com/index.php?title=Nintendo_NES_Core_Compatibility), like [`libretro-fceumm`](http://wiki.libretro.com/index.php?title=FCEUmm).
``` xml
<system>
  <name>nes</name>
  <fullname>Nintendo Entertainment System</fullname>
  <path>/path/to/roms</path>
  <extension>.nes .NES .zip</extension>
  <command>retroarch --fullscreen -L /usr/lib/libretro/fceumm_libretro.so %ROM%</command>
  <platform>nes</platform>
  <theme>nes</theme>
</system>
```

### [Mednafen](http://mednafen.sourceforge.net/)
``` xml
<system>
  <name>nes</name>
  <fullname>Nintendo Entertainment System</fullname>
  <path>/path/to/roms</path>
  <extension>.nes .NES .zip</extension>
  <command>mednafen -video.fs 1 %ROM%</command>
  <platform>nes</platform>
  <theme>nes</theme>
</system>
```


## Super Nintendo Entertainment System

### [ZSNES](http://zsnes.com/)
``` xml
<system>
  <name>snes</name>
  <fullname>Super Nintendo Entertainment System</fullname>
  <path>/path/to/roms</path>
  <extension>.smc .sfc .swc .fig .mgd .mgh .ufo .bin .gd3 .gd7 .usa .eur .jap .aus .st .bs .dx2 .048 .058 .078 .1 .a .gz .zip .jma</extension>
  <command>zsnes -m %ROM%</command>
  <platform>snes</platform>
  <theme>snes</theme>
</system>
```

### [RetroArch](http://libretro.com)
Requires a [Nintendo SNES Core](http://wiki.libretro.com/index.php?title=Nintendo_SNES_Core_Compatibility), like [`libretro-snes9x-next`](http://wiki.libretro.com/index.php?title=SNES9x_Next).
``` xml
<system>
  <name>snes</name>
  <fullname>Super Nintendo Entertainment System</fullname>
  <path>/path/to/roms</path>
  <extension>.smc .sfc .fig .bin .zip</extension>
  <command>retroarch --fullscreen -L /usr/lib/libretro/snes9x_next_libretro.so %ROM%</command>
  <platform>snes</platform>
  <theme>snes</theme>
</system>
```

## Atari 2600

### [Stella](http://stella.sourceforge.net/)
``` xml
<system>
  <name>atari2600</name>
  <fullname>Atari 2600</fullname>
  <path>/path/to/roms</path>
  <extension>.bin .zip</extension>
  <command>stella %ROM%</command>
  <platform>atari2600</platform>
  <theme>atari2600</theme>
</system>
```

## Nintendo GameBoy Advance

### [Mednafen](http://mednafen.sourceforge.net/)
``` xml
<system>
  <name>gba</name>
  <fullname>Nintendo GameBoy Advance</fullname>
  <path>/path/to/roms</path>
  <extension>.gba .zip</extension>
  <command>mednafen -video.fs 1 %ROM%</command>
  <platform>gba</platform>
  <theme>gba</theme>
</system>
```

### [VisualBoyAdvance](http://sourceforge.net/projects/vba/)
``` xml
<system>
  <name>gba</name>
  <fullname>Nintendo GameBoy Advance</fullname>
  <path>/path/to/roms</path>
  <extension>.gba .zip</extension>
  <command>VisualBoyAdvance -F %ROM%</command>
  <platform>gba</platform>
  <theme>gba</theme>
</system>
```

### [RetroArch](http://libretro.com)
Requires a [Nintendo GameBoy Advance Core](http://wiki.libretro.com/index.php?title=Nintendo_Game_Boy_Advance_Core_Compatibility), like [`libretro-vba-next`](http://wiki.libretro.com/index.php?title=VBA_Next).
``` xml
<system>
  <name>gba</name>
  <fullname>Nintendo GameBoy Advance</fullname>
  <path>/path/to/roms</path>
  <extension>.gba .zip</extension>
  <command>retroarch --fullscreen -L /usr/lib/libretro/vba_next_libretro.so %ROM%</command>
  <platform>gba</platform>
  <theme>gba</theme>
</system>
```


## [Steam](http://store.steampowered.com)

1. Create a folder at `~/.emulationstation/steam`
  ```
  mkdir -p ~/.emulationstation/steam
  ```
2. Add text files for each Steam game you would like to have available through
EmulationStation where:
  * The file name represents the game's name
  * The contents of the file represents the Steam application ID (found from the
    game's Steam store page)

  ```
  cd ~/.emulationstation/steam
  echo "250900" >> "The Binding of Isaac: Rebirth.txt"
  ```

``` xml
<system>
  <name>steam</name>
  <fullname>Steam</fullname>
  <path>~/.emulationstation/steam</path>
  <extension>.txt</extension>
  <command>steam steam://rungameid/$(tail %ROM%)</command>
  <platform>pc</platform>
  <theme>steam</theme>
</system>
```


## [PlayStation Portable](http://en.wikipedia.org/wiki/PlayStation_Portable)

### [PPSSPP](http://www.ppsspp.org)
``` xml
<system>
  <name>psp</name>
  <fullname>PlayStation Portable</fullname>
  <path>/path/to/roms</path>
  <extension>.iso</extension>
  <command>ppsspp --fullscreen --escape-exit "%ROM_RAW%"</command>
  <platform>psp</platform>
  <theme>psp</theme>
</system>
```
