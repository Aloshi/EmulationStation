#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "GuiComponent.h"
#include <vector>
#include "resources/Font.h"
#include "InputManager.h"

class ViewController;
class HelpComponent;
class ImageComponent;

class Window
{
public:
	Window();
	~Window();

	void pushGui(GuiComponent* gui);
	void removeGui(GuiComponent* gui);
	GuiComponent* peekGui();

	void textInput(const char* text);
	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	bool init(unsigned int width = 0, unsigned int height = 0);
	void deinit();

	inline ViewController* getViewController() { return mViewController; }

	void normalizeNextUpdate();

	bool getAllowSleep();
	void setAllowSleep(bool sleep);
	
	void renderLoadingScreen();

	void renderHelpPromptsEarly(); // used by ViewController to render HelpPrompts before a fade
	void setHelpPrompts(const std::vector<HelpPrompt>& prompts);

private:
	ViewController* mViewController;
	HelpComponent* mHelp;
	ImageComponent* mBackgroundOverlay;

	std::vector<GuiComponent*> mGuiStack;

	std::vector< std::shared_ptr<Font> > mDefaultFonts;

	int mFrameTimeElapsed;
	int mFrameCountElapsed;
	int mAverageDeltaTime;

	std::unique_ptr<TextCache> mFrameDataText;

	bool mNormalizeNextUpdate;

	bool mAllowSleep;
	bool mRenderedHelpPrompts;
};

#endif
