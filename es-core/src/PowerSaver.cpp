#include "PowerSaver.h"
#include "Settings.h"
#include <string.h>

bool PowerSaver::mState = false;
bool PowerSaver::mRunningScreenSaver = false;

int PowerSaver::mPlayNextTimeout = -1;
int PowerSaver::mScreenSaverTimeout = -1;
PowerSaver::mode PowerSaver::mMode = PowerSaver::DISABLED;

void PowerSaver::init()
{
	setState(true);
	updateMode();
}

int PowerSaver::getTimeout()
{
	// Used only for SDL_WaitEventTimeout. Use `getMode()` for modes.
	return mRunningScreenSaver ? mPlayNextTimeout : mScreenSaverTimeout;
}

void PowerSaver::updateTimeouts()
{
	mScreenSaverTimeout = (unsigned int) Settings::getInstance()->getInt("ScreenSaverTime");
	mScreenSaverTimeout = mScreenSaverTimeout > 0 ? mScreenSaverTimeout - getMode() : -1;
	mPlayNextTimeout = 30000 - getMode();
}

PowerSaver::mode PowerSaver::getMode()
{
	return mMode;
}

void PowerSaver::updateMode()
{
	std::string mode = Settings::getInstance()->getString("PowerSaverMode");

	if (mode == "disabled") {
		mMode = DISABLED;
	} else if (mode == "instant") {
		mMode = INSTANT;
	} else if (mode == "enhanced") {
		mMode = ENHANCED;
	} else {
		mMode = DEFAULT;
	}
	updateTimeouts();
}

bool PowerSaver::getState()
{
	return mState;
}

void PowerSaver::setState(bool state)
{
	bool ps_enabled = Settings::getInstance()->getString("PowerSaverMode") != "disabled";
	mState = ps_enabled && state;
}

void PowerSaver::runningScreenSaver(bool state)
{
	mRunningScreenSaver = state;
}

bool PowerSaver::isScreenSaverActive()
{
	return mRunningScreenSaver;
}
