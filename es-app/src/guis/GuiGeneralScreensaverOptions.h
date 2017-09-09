#ifndef _GUI_GENERAL_SCREENSAVER_OPTIONS_H_
#define _GUI_GENERAL_SCREENSAVER_OPTIONS_H_

#include "components/MenuComponent.h"
#include "GuiScreensaverOptions.h"

class GuiGeneralScreensaverOptions : public GuiScreensaverOptions
{
public:
	GuiGeneralScreensaverOptions(Window* window, const char* title);
	virtual ~GuiGeneralScreensaverOptions();

private:
	void openVideoScreensaverOptions();
	void openSlideshowScreensaverOptions();
};

#endif // _GUI_GENERAL_SCREENSAVER_OPTIONS_H_
