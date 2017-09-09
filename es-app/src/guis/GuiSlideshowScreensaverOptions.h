#ifndef _GUI_SLIDESHOW_SCREENSAVER_OPTIONS_H_
#define _GUI_SLIDESHOW_SCREENSAVER_OPTIONS_H_

#include "components/MenuComponent.h"
#include "GuiScreensaverOptions.h"

class GuiSlideshowScreensaverOptions : public GuiScreensaverOptions
{
public:
	GuiSlideshowScreensaverOptions(Window* window, const char* title);
	virtual ~GuiSlideshowScreensaverOptions();

private:
	void addWithLabel(ComponentListRow row, const std::string label, std::shared_ptr<GuiComponent> component);
	void addEditableTextComponent(ComponentListRow row, const std::string label, std::shared_ptr<GuiComponent> ed, std::string value);
};

#endif // _GUI_SLIDESHOW_SCREENSAVER_OPTIONS_H_
