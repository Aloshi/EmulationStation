#pragma once
#ifndef ES_CORE_CECINPUT_H
#define ES_CORE_CECINPUT_H

namespace CEC { class ICECAdapter; }
class Window;

class CECInput
{
public:

	~CECInput();

	static void init();
	static void deinit();

private:

	CECInput();
	static CECInput* sInstance;

	CEC::ICECAdapter* mlibCEC;

};

#endif // ES_CORE_CECINPUT_H
