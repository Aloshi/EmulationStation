class PowerSaver
{
public:
	enum mode : int { DISABLED = -1, INSTANT = 200, ENHANCED = 3000, DEFAULT = 10000 };

	static void init();

	static int getTimeout();
	static void updateTimeouts();

	static mode getMode();
	static void updateMode();

	static bool getState();
	static void setState(bool state);

	static void runningScreenSaver(bool state);

private:
	static bool mState;
	static bool mRunningScreenSaver;

	static mode mMode;
	static int mPlayNextTimeout;
	static int mScreenSaverTimeout;
};
