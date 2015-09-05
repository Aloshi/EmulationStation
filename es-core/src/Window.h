#pragma once

#include "GuiComponent.h"
#include <vector>
#include "resources/Font.h"
#include "InputManager.h"

class HelpComponent;
class ImageComponent;

class Window
{
public:
	Window();
	~Window();

	void pushGui(GuiComponent* gui);
	void displayMessage(std::string message);
	void removeGui(GuiComponent* gui);
	GuiComponent* peekGui();

	void textInput(const char* text);
	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

    bool init(unsigned int width = 0, unsigned int height = 0, bool initRenderer = true);
	void deinit();

	void normalizeNextUpdate();

	inline bool isSleeping() const { return mSleeping; }
	bool getAllowSleep();
	void setAllowSleep(bool sleep);
	
	void renderLoadingScreen();

	void renderHelpPromptsEarly(); // used to render HelpPrompts before a fade
	void setHelpPrompts(const std::vector<HelpPrompt>& prompts, const HelpStyle& style);

private:
	void onSleep();
	void onWake();

	HelpComponent* mHelp;
	ImageComponent* mBackgroundOverlay;

	std::vector<GuiComponent*> mGuiStack;
	std::vector<std::string> mMessages;

	std::vector< std::shared_ptr<Font> > mDefaultFonts;

	int mFrameTimeElapsed;
	int mFrameCountElapsed;
	int mAverageDeltaTime;

	std::unique_ptr<TextCache> mFrameDataText;

	bool mNormalizeNextUpdate;

	bool mAllowSleep;
	bool mSleeping;
	unsigned int mTimeSinceLastInput;

	bool mRenderedHelpPrompts;
        
	bool launchKodi;
};
