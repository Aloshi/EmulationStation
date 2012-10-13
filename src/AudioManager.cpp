#include "AudioManager.h"

#include "SDL.h"
#include "SDL_mixer.h"
#include <iostream>
#include "Sound.h"
#include <vector>

namespace AudioManager
{
	std::vector<Sound*> sSoundVector;

	bool sInitialized = false;

	void init()
	{
		int result = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16SYS, 2, 1024);

		if(result == -1)
		{
			std::cerr << "Error initializing AudioManager!\n";
			std::cerr << "	" << Mix_GetError() << "\n";
			return;
		}

		sInitialized = true;

		for(unsigned int i = 0; i < sSoundVector.size(); i++)
		{
			sSoundVector.at(i)->init();
		}
	}

	void registerSound(Sound* sound)
	{
		sSoundVector.push_back(sound);
	}

	void unregisterSound(Sound* sound)
	{
		for(unsigned int i = 0; i < sSoundVector.size(); i++)
		{
			if(sSoundVector.at(i) == sound)
			{
				sSoundVector.erase(sSoundVector.begin() + i);
				return;
			}
		}

		std::cerr << "AudioManager Error - tried to unregister a sound that wasn't registered!\n";
	}

	void test()
	{
		Mix_Chunk* sound = NULL;
		sound = Mix_LoadWAV("test.wav");

		if(sound == NULL)
		{
			std::cerr << "Error loading test sound!\n";
			std::cerr << "	" << Mix_GetError() << "\n";
			return;
		}

		int channel = -1;

		//third argument is play count, -1 = infinite loop, 0 = once
		channel = Mix_PlayChannel(-1, sound, 0);

		if(channel == -1)
		{
			std::cerr << "Error playing sound!\n";
			std::cerr << "	" << Mix_GetError() << "\n";
			return;
		}

		while(Mix_Playing(channel) != 0);
		Mix_FreeChunk(sound);

	}

	void deinit()
	{
		for(unsigned int i = 0; i < sSoundVector.size(); i++)
		{
			sSoundVector.at(i)->deinit();
		}

		Mix_CloseAudio();

		sInitialized = false;
	}

	bool isInitialized()
	{
		return sInitialized;
	}
}
