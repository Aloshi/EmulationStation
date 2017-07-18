#pragma once

#include "GuiComponent.h"
#include "SystemData.h"
#include "components/MenuComponent.h"
#include "CollectionSystemManager.h"
#include "components/SwitchComponent.h"
#include "Log.h"


template<typename T>
class OptionListComponent;


class GuiCollectionSystemsOptions : public GuiComponent
{
public:
	GuiCollectionSystemsOptions(Window* window);
	~GuiCollectionSystemsOptions();
	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void initializeMenu();
	void applySettings();
	void addSystemsToMenu();
	void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
	void updateSettings(std::string newAutoSettings, std::string newCustomSettings);
	void createCollection(std::string inName);
	void exitEditMode();
	std::shared_ptr< OptionListComponent<std::string> > autoOptionList;
	std::shared_ptr< OptionListComponent<std::string> > customOptionList;
	std::shared_ptr<SwitchComponent> sortAllSystemsSwitch;
	std::shared_ptr<SwitchComponent> bundleCustomCollections;
	MenuComponent mMenu;
	SystemData* mSystem;
};
