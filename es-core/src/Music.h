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
	bool playing;

public:
        
        static std::shared_ptr<Music> getFromTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element);
        static std::shared_ptr<Music> get(const std::string& path);
        void play(bool repeat, void (* callback)());
        
	~Music();


private:
	Music(const std::string & path = "");
	static std::map< std::string, std::shared_ptr<Music> > sMap;


	void initMusic();
	void deinitMusic();
        
};

#endif
