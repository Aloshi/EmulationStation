#include "AudioManager.h"

#include "Log.h"
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
			LOG(LogError) << "Error initializing AudioManager!\n	" << Mix_GetError();
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

		LOG(LogError) << "AudioManager Error - tried to unregister a sound that wasn't registered!";
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
