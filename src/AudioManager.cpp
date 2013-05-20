#include "AudioManager.h"

#include "Log.h"


std::vector<std::shared_ptr<Sound>> AudioManager::sSoundVector;
std::shared_ptr<AudioManager> AudioManager::sInstance;
SDL_AudioSpec AudioManager::sAudioFormat;


void AudioManager::mixAudio(void *unused, Uint8 *stream, int len)
{
	bool stillPlaying = false;

	//iterate through all our samples
	std::vector<std::shared_ptr<Sound>>::const_iterator soundIt = sSoundVector.cbegin();
	while (soundIt != sSoundVector.cend())
	{
		std::shared_ptr<Sound> sound = *soundIt;
		if(sound->isPlaying())
		{
			//calculate rest length of current sample
			Uint32 restLength = (sound->getLength() - sound->getPosition());
			if (restLength > len) {
				//if stream length is smaller than smaple lenght, clip it
				restLength = len;
			}
			//mix sample into stream
			SDL_MixAudio(stream, &(sound->getData()[sound->getPosition()]), restLength, SDL_MIX_MAXVOLUME);
			if (sound->getPosition() + restLength >= sound->getLength())
			{
				//if the position is beyond the end of the buffer, stop playing the sample
				sound->stop();
			}
			else 
			{
				//sample hasn't ended yet
				sound->setPosition(sound->getPosition() + restLength);
				stillPlaying = true;
			}
		}
		//advance to next sound
		++soundIt;
	}
	//we have processed all samples. check if some will still be playing
	if (!stillPlaying) {
		//no. pause audio till a Sound::play() wakes us up
		SDL_PauseAudio(1);
	}
}

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
	deinit();
}

void AudioManager::init()
{
	//Set up format and callback. Play 16-bit stereo audio at 44.1Khz
	sAudioFormat.freq = 44100;
	sAudioFormat.format = AUDIO_S16;
	sAudioFormat.channels = 2;
	sAudioFormat.samples = 1024;
	sAudioFormat.callback = mixAudio;
	sAudioFormat.userdata = NULL;

	//Open the audio device and pause
	if (SDL_OpenAudio(&sAudioFormat, NULL) < 0) {
		LOG(LogError) << "AudioManager Error - Unable to open SDL audio: " << SDL_GetError() << std::endl;
	}
}

void AudioManager::deinit()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

void AudioManager::registerSound(std::shared_ptr<Sound> & sound)
{
	//check if an AudioManager instance is already created, if not create one
	if (sInstance == nullptr) {
		sInstance = std::shared_ptr<AudioManager>(new AudioManager);
	}
	sSoundVector.push_back(sound);
}

void AudioManager::unregisterSound(std::shared_ptr<Sound> & sound)
{
	for(unsigned int i = 0; i < sSoundVector.size(); i++)
	{
		if(sSoundVector.at(i) == sound)
		{
			sSoundVector.erase(sSoundVector.begin() + i);
			return;
		}
	}
	LOG(LogError) << "AudioManager Error - tried to unregister a sound that wasn't registered!";
}

void AudioManager::play()
{
	//unpause audio, the mixer will figure out if samples need to be played...
	SDL_PauseAudio(0);
}
