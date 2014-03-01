#pragma once

#include "../GuiComponent.h"
#include "../components/ComponentGrid.h"
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
	~GuiSettingsMenu();

	bool input(InputConfig* config, Input input) override;

	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void loadStates();
	void applyStates();

	ComponentGrid mList;

	NinePatchComponent mBox;

	SwitchComponent mDrawFramerateSwitch;
	SliderComponent mVolumeSlider;
	SwitchComponent mDisableSoundsSwitch;
	OptionListComponent< std::shared_ptr<Scraper> > mScraperOptList;
	SwitchComponent mScrapeRatingsSwitch;
	SliderComponent mDimSlider;
	SwitchComponent mDisableHelpSwitch;
	ButtonComponent mSaveButton;
	
	std::vector<GuiComponent*> mLabels;
};
