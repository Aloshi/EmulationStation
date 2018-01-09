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
		"fds", // Famicom Disk System
		"nes", // nintendo entertainment system
		"gb", // game boy
		"gba", // game boy advance
		"gbc", // game boy color
		"gc", // gamecube
		"wii",
		"wiiu",
		"virtualboy",
		"gameandwatch",
		"pc",
		"sega32x",
		"segacd",
		"dreamcast",
		"gamegear",
		"genesis", // sega genesis
		"mastersystem", // sega master system
		"megadrive", // sega megadrive
		"saturn", // sega saturn
		"sg-1000",
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
		"videopac",
		"vectrex",
		"trs-80",
		"coco",

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

	int getMameTitleCount()
	{
		const char** mameNames = mameNameToRealName;
		int count = 0;
		while (*mameNames != NULL)
		{
			mameNames += 2;
			count++;
		}
		return count;
	}


	const char* mameTitleSearch(const char* from)
	{
		// The start and end index range from [0, number of roms]
		int iStart = 0;
		static int mameCount = getMameTitleCount();
		int iEnd = mameCount;

		while (iStart < iEnd)
		{
			// The middle entry is halfway between the start and end index
			const int iMiddle = (iStart + iEnd) / 2;

			// mameNameToRealName contains 2 sequential entries for every entry, so the indexes look like this:
			// 0: key, value,
			// 2: key, value,
			// 4: key, value
			// This means that there are twice as many indexes as there are numbers of ROMs. So to get the
			// iMiddle'th entry, we need to multiply by 2 because iMiddle goes from [0, number of roms].
			const int iKey = iMiddle * 2;
			const int comp = strcmp(mameNameToRealName[iKey], from);
			if (comp < 0)
			{
				// Remember, iMiddle ranges from [0, number of roms] so we only increment by 1
				iStart = iMiddle + 1;
			}
			else if (comp > 0)
			{
				iEnd = iMiddle;
			}
			// The Key was found, now return the Value
			else return mameNameToRealName[iKey + 1];
		}
		return from;
	}

}
