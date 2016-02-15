#ifndef _AUDIOMANAGER_H_
#define _AUDIOMANAGER_H_

#include <vector>
#include <memory>

#include "SDL_audio.h"

#include "Sound.h"
#include "Music.h"


class AudioManager {
    static std::vector<std::shared_ptr<Sound>> sSoundVector;
    static std::vector<std::shared_ptr<Music>> sMusicVector;

    static std::shared_ptr<AudioManager> sInstance;
    std::shared_ptr<Music> currentMusic;


    AudioManager();

public:
    static std::shared_ptr<AudioManager> &getInstance();

    void stopMusic();

    void themeChanged(const std::shared_ptr<ThemeData> &theme);

    void resumeMusic();

    void init();

    void deinit();

    void registerMusic(std::shared_ptr<Music> &music);

    void registerSound(std::shared_ptr<Sound> &sound);

    void unregisterMusic(std::shared_ptr<Music> &music);

    void unregisterSound(std::shared_ptr<Sound> &sound);

    void play();

    void stop();

    void musicEnd();

    virtual ~AudioManager();

private:
    bool running;
    int lastTime = 0;

    std::shared_ptr<Music> getRandomMusic(std::string themeMusicDirectory);

    bool runningFromPlaylist;

    bool update(int curTime);

    std::string currentThemeMusicDirectory;

    void playRandomMusic();
};

#endif
