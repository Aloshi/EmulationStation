#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include <functional>
#include <Window.h>
#include <SystemData.h>
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
	void createInputTextRow(GuiSettings * gui, const char* title, const char* settingsID, bool password);
	MenuComponent mMenu;
	TextComponent mVersion;


	std::shared_ptr<OptionListComponent<std::string>> createRatioOptionList(Window *window,
                                                                        std::string configname) const;

	void popSystemConfigurationGui(SystemData *systemData, std::string previouslySelectedEmulator) const;
};
