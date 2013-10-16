#ifndef _SETTINGSMENU_H_
#define _SETTINGSMENU_H_

#include "../GuiComponent.h"
#include "ComponentListComponent.h"
#include <vector>
#include "SwitchComponent.h"
#include "SliderComponent.h"
#include "TextComponent.h"
#include "NinePatchComponent.h"
#include "OptionListComponent.h"
#include "ButtonComponent.h"
#include "../scrapers/Scraper.h"

class GuiSettingsMenu : public GuiComponent
{
public:
	GuiSettingsMenu(Window* window);
	~GuiSettingsMenu();

	bool input(InputConfig* config, Input input) override;

private:
	void loadStates();
	void applyStates();

	ComponentListComponent mList;

	NinePatchComponent mBox;

	SwitchComponent mDrawFramerateSwitch;
	SliderComponent mVolumeSlider;
	SwitchComponent mDisableSoundsSwitch;
	OptionListComponent< std::shared_ptr<Scraper> > mScraperOptList;
	SwitchComponent mScrapeRatingsSwitch;
	ButtonComponent mSaveButton;
	
	std::vector<GuiComponent*> mLabels;
};

#endif
