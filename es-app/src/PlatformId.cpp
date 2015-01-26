#include "PlatformId.h"
#include <string.h>

extern const char* mameNameToRealName[];

namespace PlatformIds
{
	const char* PlatformNames[PLATFORM_COUNT + 1] = {
		"unknown", // nothing set

		"3do",
		"amiga",
		"amstradcpc",
		"apple2",
		"arcade",
		"atari800",
		"atari2600",
		"atari5200",
		"atari7800",
		"atarilynx",
		"atarist",
		"atarijaguar",
		"atarijaguarcd",
		"atarixe",
		"colecovision",
		"c64", // commodore 64
		"intellivision",
		"macintosh",
		"xbox",
		"xbox360",
		"msx",
		"neogeo",
		"ngp", // neo geo pocket
		"ngpc", // neo geo pocket color
		"n3ds", // nintendo 3DS
		"n64", // nintendo 64
		"nds", // nintendo DS
		"nes", // nintendo entertainment system
		"gb", // game boy
		"gba", // game boy advance
		"gbc", // game boy color
		"gc", // gamecube
		"wii",
		"wiiu",
		"pc",
		"sega32x",
		"segacd",
		"dreamcast",
		"gamegear",
		"genesis", // sega genesis
		"mastersystem", // sega master system
		"megadrive", // sega megadrive
		"saturn", // sega saturn
		"psx",
		"ps2",
		"ps3",
		"ps4",
		"psvita",
		"psp", // playstation portable
		"snes", // super nintendo entertainment system
		"pcengine", // turbografx-16/pcengine
		"wonderswan",
		"wonderswancolor",
		"zxspectrum",

		"ignore", // do not allow scraping for this system
		"invalid"
	};

	PlatformId getPlatformId(const char* str)
	{
		if(str == NULL)
			return PLATFORM_UNKNOWN;

		for(unsigned int i = 1; i < PLATFORM_COUNT; i++)
		{
			if(strcmp(PlatformNames[i], str) == 0)
				return (PlatformId)i;
		}

		return PLATFORM_UNKNOWN;
	}

	const char* getPlatformName(PlatformId id)
	{
		return PlatformNames[id];
	}

	const char* getCleanMameName(const char* from)
	{
		const char** mameNames = mameNameToRealName;

		while(*mameNames != NULL && strcmp(from, *mameNames) != 0)
			mameNames += 2;

		if(*mameNames)
			return *(mameNames + 1);
		
		return from;
	}
}
