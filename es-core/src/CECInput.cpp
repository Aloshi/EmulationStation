#include "CECInput.h"

#ifdef HAVE_LIBCEC
#include "Log.h"
#include <libcec/cec.h>
#include <iostream> // bad bad cecloader
#include <libcec/cecloader.h>
#include <SDL_events.h>
#endif // HAVE_LIBCEC
#include <assert.h>

// hack for cec support
extern int SDL_USER_CECBUTTONDOWN;
extern int SDL_USER_CECBUTTONUP;

CECInput* CECInput::sInstance = nullptr;

#ifdef HAVE_LIBCEC
static int onAlert(void* /*cbParam*/, const CEC::libcec_alert /*type*/, const CEC::libcec_parameter /*param*/)
{
	return 0;
}

static int onCommand(void* /*cbParam*/, const CEC::cec_command /*command*/)
{
	return 0;
}

static int onKeyPress(void* /*cbParam*/, const CEC::cec_keypress key)
{
	SDL_Event event;
	event.type      = (key.duration > 0) ? SDL_USER_CECBUTTONUP : SDL_USER_CECBUTTONDOWN;
	event.user.code = key.keycode;
	SDL_PushEvent(&event);

	return 0;
}

static int onLogMessage(void* /*cbParam*/, const CEC::cec_log_message /*message*/)
{
	return 0;
}
#endif // HAVE_LIBCEC

void CECInput::init()
{
	assert(!sInstance);
	sInstance = new CECInput();
}

void CECInput::deinit()
{
	assert(sInstance);
	delete sInstance;
	sInstance = nullptr;
}

CECInput::CECInput() : mlibCEC(nullptr)
{

#ifdef HAVE_LIBCEC
	CEC::ICECCallbacks        callbacks;
	CEC::libcec_configuration config;
	config.Clear();
	callbacks.Clear();

	sprintf(config.strDeviceName, "RetroPie ES");
	config.clientVersion   = CEC::LIBCEC_VERSION_CURRENT;
	config.bActivateSource = 0;
	config.callbacks       = &callbacks;
	config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_PLAYBACK_DEVICE);

	callbacks.CBCecAlert      = &onAlert;
	callbacks.CBCecCommand    = &onCommand;
	callbacks.CBCecKeyPress   = &onKeyPress;
	callbacks.CBCecLogMessage = &onLogMessage;

	mlibCEC = LibCecInitialise(&config);
	
	if(!mlibCEC)
	{
		LOG(LogInfo) << "CECInput::LibCecInitialise failed";
		return;
	}

	CEC::cec_adapter adapters[10];
	int8_t numAdapters = mlibCEC->FindAdapters(adapters, 10, nullptr);

	if(numAdapters <= 0)
	{
		LOG(LogInfo) << "CECInput::mAdapter->FindAdapters failed";
		UnloadLibCec(mlibCEC);
		mlibCEC = nullptr;
		return;
	}

	for(int i = 0; i < numAdapters; ++i)
		LOG(LogDebug) << "adapter: " << i << " path: " << adapters[i].comm  << " comm: " << adapters[i].comm;

	if(!mlibCEC->Open(adapters[0].comm))
	{
		LOG(LogInfo) << "CECInput::mAdapter->Open failed";
		UnloadLibCec(mlibCEC);
		mlibCEC = nullptr;
		return;
	}

	LOG(LogDebug) << "CECInput succeeded";
#endif // HAVE_LIBCEC

}

CECInput::~CECInput()
{

#ifdef HAVE_LIBCEC
	if(mlibCEC)
	{
		mlibCEC->Close();
		UnloadLibCec(mlibCEC);
		mlibCEC = nullptr;
	}
#endif // HAVE_LIBCEC

}
