#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "GuiComponent.h"
#include "InputManager.h"
#include <vector>
#include "resources/Font.h"

class ViewController;
class HelpComponent;

class Window
{
public:
	Window();
	~Window();

	void pushGui(GuiComponent* gui);
	void removeGui(GuiComponent* gui);
	GuiComponent* peekGui();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	bool init(unsigned int width = 0, unsigned int height = 0);
	void deinit();

	inline InputManager* getInputManager() { return mInputManager; }
	inline ViewController* getViewController() { return mViewController; }

	void normalizeNextUpdate();

	bool getAllowSleep();
	void setAllowSleep(bool sleep);
	
	void renderLoadingScreen();

	void setHelpPrompts(const std::vector<HelpPrompt>& prompts);

private:
	InputManager* mInputManager;
	ViewController* mViewController;
	HelpComponent* mHelp;
	std::vector<GuiComponent*> mGuiStack;

	std::vector< std::shared_ptr<Font> > mDefaultFonts;

	int mFrameTimeElapsed;
	int mFrameCountElapsed;
	int mAverageDeltaTime;
	std::string mFrameDataString;

	bool mNormalizeNextUpdate;

	bool mAllowSleep;
};

#endif
