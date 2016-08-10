#pragma once

#include <memory>
#include <cstdint>

/*!
Singleton pattern. Call getInstance() to get an object.
*/
class VolumeControl
{
	int internalVolume;

	static std::weak_ptr<VolumeControl> sInstance;

	VolumeControl();
	VolumeControl(const VolumeControl & right);
    VolumeControl & operator=(const VolumeControl & right);

public:
	static std::shared_ptr<VolumeControl> & getInstance();

	void init();
	void deinit();

	int getVolume() const;
	void setVolume(int volume);

	~VolumeControl();
};
