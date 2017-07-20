#include "PowerSaver.h"
#include "Settings.h"
#include <string.h>

bool PowerSaver::mState = true;
int PowerSaver::mTimeout = PowerSaver::ps_default;

void PowerSaver::init(bool state)
{
	setState(true);
	updateTimeout();
}

int PowerSaver::getTimeout()
{
	return mTimeout;
}

void PowerSaver::updateTimeout()
{
	std::string mode = Settings::getInstance()->getString("PowerSaverMode");
	
	if (mode == "disabled") {
		mTimeout = ps_disabled;
	} else if (mode == "instant") {
		mTimeout = ps_instant;
	} else if (mode == "enhanced") {
		mTimeout = ps_enhanced;
	} else { // default
		mTimeout = ps_default;
	}
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

