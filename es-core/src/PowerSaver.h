class PowerSaver
{
public:
	enum ps_state : int { ps_disabled = -1, ps_instant = 200, ps_enhanced = 3000, ps_default = 10000 };
	
	static void init(bool state = true);
	
	static int getTimeout();
	static void updateTimeout();

	static bool getState();
	static void setState(bool state);
	
private:
	static bool mState;
	static int mTimeout;
};
