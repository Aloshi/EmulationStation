#include "PlatformId.h"
#include <string.h>

extern const char* mameNameToRealName[];

namespace PlatformIds
{
	const char* PlatformNames[PLATFORM_COUNT + 1] = {
		"unknown", // = 0,

		"3do", // = 1,
		"amiga", // = 2,
		"arcade", // = 3,
		"atari2600", // = 4,
		"atari5200", // = 5,
		"atari7800", // = 6,
		"atariJaguar", // = 7,
		"atariJaguarCD", // = 8,
		"atariXE", // = 9,
		"colecovision", // = 10,
		"commodore64", // = 11,
		"intellivision", // = 12,
		"mac", // = 13,
		"xbox", // = 14,
		"xbox360", // = 15,
		"neogeo", // = 16,
		"ngp", // = 17,
		"ngpc", // = 18,
		"n3ds", // = 19,
		"n64", // = 20,
		"nds", // = 21,
		"nes", // = 22,
		"gb", // = 23,
		"gba", // = 24,
		"gbc", // = 25,
		"gamecube", // = 26,
		"wii", // = 27,
		"wiiu", // = 28,
		"pc", // = 29,
		"sega32x", // = 30,
		"segacd", // = 31,
		"dreamcast", // = 32,
		"gamegear", // = 33,
		"genesis", // = 34,
		"mastersystem", // = 35,
		"megadrive", // = 36,
		"saturn", // = 37,
		"psx", // = 38,
		"ps2", // = 39,
		"ps3", // = 40,
		"ps4", // = 41,
		"psvita", // = 42,
		"psp", // = 43,
		"snes", // = 44,
		"pcengine", // = 45,
		"zxspectrum", // = 46,

		"ignore", // = 47 // do not allow scraping for this system
		"invalid" // = 48
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
