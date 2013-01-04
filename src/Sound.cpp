#include "Sound.h"
#include <iostream>
#include "AudioManager.h"
#include "Log.h"

Sound::Sound(std::string path)
{
	mSound = NULL;
	mChannel = -1;

	AudioManager::registerSound(this);

	loadFile(path);
}

Sound::~Sound()
{
	deinit();

	AudioManager::unregisterSound(this);
}

void Sound::loadFile(std::string path)
{
	mPath = path;
	init();
}

void Sound::init()
{
	if(!AudioManager::isInitialized())
		return;

	if(mSound != NULL)
		deinit();

	if(mPath.empty())
		return;

	mSound = Mix_LoadWAV(mPath.c_str());

	if(mSound == NULL)
	{
		LOG(LogError) << "Error loading sound \"" << mPath << "\"!\n" << "	" << Mix_GetError();
	}
}

void Sound::deinit()
{
	if(mSound != NULL)
	{
		Mix_FreeChunk(mSound);
		mSound = NULL;
	}
}

void Sound::play()
{
	if(mSound == NULL)
		return;

	mChannel = -1;

	mChannel = Mix_PlayChannel(-1, mSound, 0);
	if(mChannel == -1)
	{
		LOG(LogError) << "Error playing sound!\n	" << Mix_GetError();
	}
}

bool Sound::isPlaying()
{
	if(mChannel != -1 && Mix_Playing(mChannel))
		return true;
	else
		return false;
}

