#include "Music.h"
#include "Log.h"
#include "Settings.h"
#include "ThemeData.h"

std::map< std::string, std::shared_ptr<Music> > Music::sMap;
std::shared_ptr<Music>  Music::currentMusic = NULL;
void Music::init()
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	{
		LOG(LogError) << "Error initializing SDL audio!\n" << SDL_GetError();
		return;
	}

	//Open the audio device and pause
        if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 ) < 0 ){
		LOG(LogError) << "MUSIC Error - Unable to open SDLMixer audio: " << SDL_GetError() << std::endl;
	}
}

void Music::deinit()
{
	Mix_HaltMusic();
	//completely tear down SDL audio. else SDL hogs audio resources and emulators might fail to start...
	// TODO FOREACH MUSIC CLEAN
        Mix_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void Music::stopMusic()
{
       Mix_FadeOutMusic(1000);
    
       Mix_HaltMusic();
       Music::currentMusic = NULL;
}

void Music::startMusic(const std::shared_ptr<ThemeData>& theme)
{
    std::shared_ptr<Music> bgsound = Music::getFromTheme(theme,"system", "bgsound");
    if(bgsound){
         Music::stopMusic();
         bgsound->play();
         Music::currentMusic = bgsound;
    }else {
        LOG(LogError) << "NO SOUND FOUND";
        Music::stopMusic();
    }
}

void Music::resume(){
    Music::init();
    if(Music::currentMusic != NULL){
        Music::currentMusic->play();
    }
}

std::shared_ptr<Music> Music::get(const std::string& path)
{
	auto it = sMap.find(path);
	if(it != sMap.end())
		return it->second;
	std::shared_ptr<Music> music = std::shared_ptr<Music>(new Music(path));
	sMap[path] = music;
	return music;
}

std::shared_ptr<Music> Music::getFromTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element)
{
	LOG(LogInfo) << " req music [" << view << "." << element << "]";
	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "sound");
	if(!elem || !elem->has("path"))
	{
		LOG(LogInfo) << "   (missing)";
		return NULL;
	}
	return get(elem->get<std::string>("path"));
}

Music::Music(const std::string & path) : music(NULL), playing(false)
{
	mPath = path;
	initMusic();
}

Music::~Music()
{
	deinitMusic();
}


void Music::initMusic()
{
	if(music != NULL)
		deinit();

	if(mPath.empty())
		return;

	//load wav file via SDL
        Mix_Music *gMusic = NULL;
        gMusic = Mix_LoadMUS( mPath.c_str() );
        if(gMusic == NULL){
            LOG(LogError) << "Error loading sound \"" << mPath << "\"!\n" << "	" << SDL_GetError();
            return;
        }else {
            music = gMusic;
        }
}

void Music::deinitMusic()
{
	playing = false;
        if(music != NULL){
            Mix_FreeMusic( music );
            music = NULL;
        }
}

void Music::play()
{

        if(music == NULL)
		return;
	if(!Settings::getInstance()->getBool("EnableSounds"))
		return;
	if (!playing)
	{
		playing = true;
	}
        LOG(LogError) << "playing";
        if(Mix_FadeInMusic(music, -1, 1000) == -1){
            printf("Mix_PlayMusic: %s\n", Mix_GetError());
        }else {
        }
}

bool Music::isPlaying() const
{
	return playing;
}
