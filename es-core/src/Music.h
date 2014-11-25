#ifndef _MUSIC_H_
#define _MUSIC_H_

#include <string>
#include <map>
#include <memory>
#include "SDL_mixer.h"

class ThemeData;

class Music
{	
        std::string mPath;
	Mix_Music * music;
	static std::shared_ptr<Music> currentMusic;
	bool playing;

public:

        static void stopMusic();
        static void startMusic(const std::shared_ptr<ThemeData>& theme);
        static void resume();
        static void init();
        static void deinit();
        
	~Music();


private:
	Music(const std::string & path = "");
	static std::map< std::string, std::shared_ptr<Music> > sMap;
        static std::shared_ptr<Music> getFromTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element);
        static std::shared_ptr<Music> get(const std::string& path);

	void initMusic();
	void deinitMusic();
        
        void play();
        bool isPlaying() const;
};

#endif
