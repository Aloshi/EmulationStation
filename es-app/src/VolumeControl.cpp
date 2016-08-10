#include "VolumeControl.h"

#include "Log.h"


std::weak_ptr<VolumeControl> VolumeControl::sInstance;


VolumeControl::VolumeControl()
	: internalVolume(100)
{
	init();
}

VolumeControl::VolumeControl(const VolumeControl & right)
{
	sInstance = right.sInstance;
}

VolumeControl & VolumeControl::operator=(const VolumeControl & right)
{
	if (this != &right) {
		sInstance = right.sInstance;
	}

	return *this;
}

VolumeControl::~VolumeControl()
{
	//set original volume levels for system
	//setVolume(originalVolume);

	deinit();
}

std::shared_ptr<VolumeControl> & VolumeControl::getInstance()
{
	//check if an VolumeControl instance is already created, if not create one
	static std::shared_ptr<VolumeControl> sharedInstance = sInstance.lock();
	if (sharedInstance == nullptr) {
		sharedInstance.reset(new VolumeControl);
		sInstance = sharedInstance;
	}
	return sharedInstance;
}

void VolumeControl::init()
{
	//initialize audio mixer interface
}

void VolumeControl::deinit()
{
	//deinitialize audio mixer interface
}

int VolumeControl::getVolume() const
{
	return internalVolume;
}

void VolumeControl::setVolume(int volume)
{
    internalVolume = volume;
}
