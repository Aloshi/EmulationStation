#ifndef _SOUND_H_
#define _SOUND_H_

#include <string>
#include <map>
#include <memory>
#include "SDL_mixer.h"

class ThemeData;

class Sound
{
	std::string mPath;
	Mix_Chunk * mSampleData;
	bool playing;

public:
	static std::shared_ptr<Sound> get(const std::string& path);
	static std::shared_ptr<Sound> getFromTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& elem);

	~Sound();

	void init();
	void deinit();

	void loadFile(const std::string & path);

	void play();
	bool isPlaying() const;
	void stop();


private:
	Sound(const std::string & path = "");
	static std::map< std::string, std::shared_ptr<Sound> > sMap;
};

#endif
