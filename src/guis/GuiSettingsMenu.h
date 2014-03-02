#pragma once

#include "../GuiComponent.h"
#include "../components/MenuComponent.h"
#include "../components/SwitchComponent.h"
#include "../components/SliderComponent.h"
#include "../components/TextComponent.h"
#include "../components/NinePatchComponent.h"
#include "../components/OptionListComponent.h"
#include "../components/ButtonComponent.h"
#include "../scrapers/Scraper.h"

class GuiSettingsMenu : public GuiComponent
{
public:
	GuiSettingsMenu(Window* window);
	
	bool input(InputConfig* config, Input input) override;

	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void addSetting(const char* label, const std::shared_ptr<GuiComponent>& comp, const std::function<void()>& saveFunc);
	void save();

	std::vector< std::function<void()> > mApplyFuncs;

	MenuComponent mMenu;
};
