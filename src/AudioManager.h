#ifndef _AUDIOMANAGER_H_
#define _AUDIOMANAGER_H_

class Sound;

namespace AudioManager
{
	void registerSound(Sound* sound);
	void unregisterSound(Sound* sound);

	bool isInitialized();

	void init();
	void deinit();
}

#endif
