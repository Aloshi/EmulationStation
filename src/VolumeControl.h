#pragma once

#include <memory>
#include <stdint.h>

#if defined (__APPLE__)
    #error TODO: Not implemented for MacOS yet!!!
#elif defined(__linux__)
	#include <unistd.h>
	#include <fcntl.h>
	#include <alsa/asoundlib.h>
#elif defined(WIN32) || defined(_WIN32)
	#include <Windows.h>
	#include <MMSystem.h>
	#include <mmdeviceapi.h>
	#include <endpointvolume.h>
#endif

/*!
Singleton pattern. Call getInstance() to get an object.
*/
class VolumeControl
{
#if defined (__APPLE__)
    #error TODO: Not implemented for MacOS yet!!!
#elif defined(__linux__)
    static const char * mixerName;
    static const char * mixerCard;
    int mixerIndex;
	snd_mixer_t* mixerHandle;
    snd_mixer_elem_t* mixerElem;
    snd_mixer_selem_id_t* mixerSelemId;
#elif defined(WIN32) || defined(_WIN32)
	HMIXER mixerHandle;
	MIXERCONTROL mixerControl;
	IAudioEndpointVolume * endpointVolume;
#endif

	uint8_t originalVolume;
	uint8_t internalVolume;

	static std::shared_ptr<VolumeControl> sInstance;

	VolumeControl();

public:
	static std::shared_ptr<VolumeControl> & getInstance();

	void init();
	void deinit();

	void getVolume(uint8_t & volume);
	void setVolume(uint8_t volume);

	virtual ~VolumeControl();
};
