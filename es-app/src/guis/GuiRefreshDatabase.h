#pragma once

#include "GuiComponent.h"
#include "SystemData.h"
#include "components/MenuComponent.h"
#include <queue>


template<typename T>
class OptionListComponent;

class SwitchComponent;

//Allows user to run the expensive file systems scans manually.
//Takes the current system to select only it by default. If nullptr, all are selected.
class GuiRefreshDatabase : public GuiComponent
{
public:
	GuiRefreshDatabase(Window* window);

	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void pressedStart();
	void start();

	std::shared_ptr< OptionListComponent<SystemData*> > mSystems;
	std::shared_ptr<SwitchComponent> mAddFiles;
	std::shared_ptr<SwitchComponent> mCheckExists;
	std::shared_ptr<SwitchComponent> mRemoveNonexisting;

	MenuComponent mMenu;
};
