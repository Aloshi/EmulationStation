#ifndef _AUDIOMANAGER_H_
#define _AUDIOMANAGER_H_

#include <vector>
#include <memory>

#include "SDL_audio.h"

#include "Sound.h"


class AudioManager
{
	static SDL_AudioSpec sAudioFormat;
	static std::vector<std::shared_ptr<Sound>> sSoundVector;
	static std::shared_ptr<AudioManager> sInstance;

	static void mixAudio(void *unused, Uint8 *stream, int len);

	AudioManager();

public:
	static std::shared_ptr<AudioManager> & getInstance();

	void init();
	void deinit();

	static void registerSound(std::shared_ptr<Sound> & sound);
	static void unregisterSound(std::shared_ptr<Sound> & sound);

	static void play();

	virtual ~AudioManager();
};

#endif
