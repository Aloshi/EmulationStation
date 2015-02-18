#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include <functional>
#include "guis/GuiSettings.h"

class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window);

	bool input(InputConfig* config, Input input) override;
	void onSizeChanged() override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
        void createConfigInput();
        void createInputTextRow(GuiSettings * gui, const char* title, const char* settingsID);
	MenuComponent mMenu;
	TextComponent mVersion;
};
