#ifndef _SOUND_H_
#define _SOUND_H_

#include <string>
#include "SDL_audio.h"


class Sound
{
	std::string mPath;
    SDL_AudioSpec mSampleFormat;
	Uint8 * mSampleData;
    Uint32 mSamplePos;
    Uint32 mSampleLength;
	bool playing;

public:
	Sound(const std::string & path = "");
	~Sound();

	void init();
	void deinit();

	void loadFile(const std::string & path);

	void play();
	bool isPlaying() const;
	void stop();

	const Uint8 * getData() const;
	Uint32 getPosition() const;
	void setPosition(Uint32 newPosition);
	Uint32 getLength() const;
	Uint32 getLengthMS() const;
};

#endif
