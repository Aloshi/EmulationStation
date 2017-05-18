#pragma once

#include "GuiComponent.h"
#include "SystemData.h"
#include "components/MenuComponent.h"
#include "FileFilterIndex.h"
#include "Log.h"


template<typename T>
class OptionListComponent;


class GuiGamelistFilter : public GuiComponent
{
public:
	GuiGamelistFilter(Window* window, SystemData* system);
	~GuiGamelistFilter();
	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void initializeMenu();
	void applyFilters();
	void resetAllFilters();
	void addFiltersToMenu();

	std::map<FilterIndexType, std::shared_ptr< OptionListComponent<std::string> >> mFilterOptions;

	MenuComponent mMenu;
	SystemData* mSystem;
	FileFilterIndex* mFilterIndex;
};
