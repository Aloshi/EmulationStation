#ifndef _SOUND_H_
#define _SOUND_H_

#include <string>
#include "SDL_mixer.h"

class Sound
{
public:
	Sound(std::string path = "");
	~Sound();

	void init();
	void deinit();

	void loadFile(std::string path);

	void play();
	bool isPlaying();
private:
	std::string mPath;
	int mChannel;
	Mix_Chunk* mSound;
};

#endif
