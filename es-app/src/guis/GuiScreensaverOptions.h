#ifndef _GUI_SCREENSAVER_OPTIONS_H_
#define _GUI_SCREENSAVER_OPTIONS_H_

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "SystemData.h"

// This is just a really simple template for a GUI that calls some save functions when closed.
class GuiScreensaverOptions : public GuiComponent
{
public:
	GuiScreensaverOptions(Window* window, const char* title);
	virtual ~GuiScreensaverOptions(); // just calls save();

	virtual void save();
	inline void addRow(const ComponentListRow& row) { mMenu.addRow(row); };
	inline void addWithLabel(const std::string& label, const std::shared_ptr<GuiComponent>& comp) { mMenu.addWithLabel(label, comp); };
	inline void addSaveFunc(const std::function<void()>& func) { mSaveFuncs.push_back(func); };

	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	HelpStyle getHelpStyle() override;

protected:
	MenuComponent mMenu;
	std::vector< std::function<void()> > mSaveFuncs;
};

#endif // _GUI_SCREENSAVER_OPTIONS_H_
