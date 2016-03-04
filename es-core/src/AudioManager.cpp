#include "AudioManager.h"

#include <SDL.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <views/SystemView.h>
#include "Log.h"
#include "RecalboxConf.h"
#include "Settings.h"
#include "ThemeData.h"

std::vector<std::shared_ptr<Sound>> AudioManager::sSoundVector;
std::vector<std::shared_ptr<Music>> AudioManager::sMusicVector;


std::shared_ptr<AudioManager> AudioManager::sInstance;


AudioManager::AudioManager() : currentMusic(NULL), running(0) {
    init();
}

AudioManager::~AudioManager() {
    deinit();
}

std::shared_ptr<AudioManager> &AudioManager::getInstance() {
    //check if an AudioManager instance is already created, if not create one
    if (sInstance == nullptr) {
        sInstance = std::shared_ptr<AudioManager>(new AudioManager);
    }
    return sInstance;
}

void AudioManager::init() {
    runningFromPlaylist = false;
    if (running == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            LOG(LogError) << "Error initializing SDL audio!\n" << SDL_GetError();
            return;
        }

        //Open the audio device and pause
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
            LOG(LogError) << "MUSIC Error - Unable to open SDLMixer audio: " << SDL_GetError() << std::endl;
        } else {
            LOG(LogInfo) << "SDL AUDIO Initialized";
            running = 1;
        }
    }
}

void AudioManager::deinit() {
    //stop all playback
    //stop();
    //completely tear down SDL audio. else SDL hogs audio resources and emulators might fail to start...
    LOG(LogInfo) << "Shutting down SDL AUDIO";

    Mix_HaltMusic();
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    running = 0;
}

void AudioManager::stopMusic() {
    Mix_FadeOutMusic(1000);
    Mix_HaltMusic();
    currentMusic = NULL;
}

void musicEndInternal() {
    AudioManager::getInstance()->musicEnd();
}

void AudioManager::themeChanged(const std::shared_ptr<ThemeData> &theme) {
    if (RecalboxConf::getInstance()->get("audio.bgmusic") == "1") {
        const ThemeData::ThemeElement *elem = theme->getElement("system", "directory", "sound");
        if (!elem || !elem->has("path")) {
            currentThemeMusicDirectory = "";
        } else {
            currentThemeMusicDirectory = elem->get<std::string>("path");
        }

        std::shared_ptr<Music> bgsound = Music::getFromTheme(theme, "system", "bgsound");

        // Found a music for the system
        if (bgsound) {
            runningFromPlaylist = false;
            stopMusic();
            bgsound->play(true, NULL);
            currentMusic = bgsound;
            return;
        }

        if (!runningFromPlaylist) {
            playRandomMusic();
        }
    }
}

void AudioManager::playRandomMusic() {// Find a random song in user directory or theme music directory
    std::shared_ptr<Music> bgsound = getRandomMusic(currentThemeMusicDirectory);
    if (bgsound) {
        runningFromPlaylist = true;
        stopMusic();
        bgsound->play(false, musicEndInternal);
        currentMusic = bgsound;
        return;
    } else {
        // Not running from playlist, and no theme song found
        stopMusic();
    }
}


void AudioManager::resumeMusic() {
    this->init();
    if (currentMusic != NULL && RecalboxConf::getInstance()->get("audio.bgmusic") == "1") {
        currentMusic->play(runningFromPlaylist ? false : true, runningFromPlaylist ? musicEndInternal : NULL);
    }
}

void AudioManager::registerSound(std::shared_ptr<Sound> &sound) {
    getInstance();
    sSoundVector.push_back(sound);
}

void AudioManager::registerMusic(std::shared_ptr<Music> &music) {
    getInstance();
    sMusicVector.push_back(music);
}

void AudioManager::unregisterSound(std::shared_ptr<Sound> &sound) {
    getInstance();
    for (unsigned int i = 0; i < sSoundVector.size(); i++) {
        if (sSoundVector.at(i) == sound) {
            sSoundVector[i]->stop();
            sSoundVector.erase(sSoundVector.begin() + i);
            return;
        }
    }
    LOG(LogError) << "AudioManager Error - tried to unregister a sound that wasn't registered!";
}

void AudioManager::unregisterMusic(std::shared_ptr<Music> &music) {
    getInstance();
    for (unsigned int i = 0; i < sMusicVector.size(); i++) {
        if (sMusicVector.at(i) == music) {
            //sMusicVector[i]->stop();
            sMusicVector.erase(sMusicVector.begin() + i);
            return;
        }
    }
    LOG(LogError) << "AudioManager Error - tried to unregister a music that wasn't registered!";
}

void AudioManager::play() {
    getInstance();

    //unpause audio, the mixer will figure out if samples need to be played...
    //SDL_PauseAudio(0);
}

void AudioManager::stop() {
    //stop playing all Sounds
    for (unsigned int i = 0; i < sSoundVector.size(); i++) {
        if (sSoundVector.at(i)->isPlaying()) {
            sSoundVector[i]->stop();
        }
    }
    //stop playing all Musics


    //pause audio
    //SDL_PauseAudio(1);
}


std::vector<std::string> getMusicIn(const std::string &path) {
    std::vector<std::string> all_matching_files;

    if (!boost::filesystem::is_directory(path)) {
        return all_matching_files;
    }
    const std::string target_path(path);
    const boost::regex my_filter(".*\\.(mp3|ogg)$");


    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(target_path); i != end_itr; ++i) {
        // Skip if not a file
        if (!boost::filesystem::is_regular_file(i->status())) continue;

        boost::smatch what;

        // Skip if no match
        if (!boost::regex_match(i->path().string(), what, my_filter)) continue;

        // File matches, store it
        all_matching_files.push_back(i->path().string());
    }
}

std::shared_ptr<Music> AudioManager::getRandomMusic(std::string themeSoundDirectory) {
    // 1 check in User music directory
    std::vector<std::string> musics = getMusicIn(Settings::getInstance()->getString("MusicDirectory"));
    if (musics.empty()) {
        //  Check in theme sound directory
        if (themeSoundDirectory != "") {
            musics = getMusicIn(themeSoundDirectory);
            if(musics.empty()) return NULL;
        } else return NULL;
    }
    int randomIndex = rand() % musics.size();
    std::shared_ptr<Music> bgsound = Music::get(musics.at(randomIndex));
    return bgsound;
}

void AudioManager::musicEnd() {
    LOG(LogInfo) << "MusicEnded";
    if (runningFromPlaylist && RecalboxConf::getInstance()->get("audio.bgmusic") == "1") {
        playRandomMusic();
    }
}
