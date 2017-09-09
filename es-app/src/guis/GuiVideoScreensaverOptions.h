#ifndef _GUI_VIDEO_SCREENSAVER_OPTIONS_H_
#define _GUI_VIDEO_SCREENSAVER_OPTIONS_H_

#include "components/MenuComponent.h"
#include "GuiScreensaverOptions.h"

class GuiVideoScreensaverOptions : public GuiScreensaverOptions
{
public:
	GuiVideoScreensaverOptions(Window* window, const char* title);
	virtual ~GuiVideoScreensaverOptions();

	void save() override;
};

#endif // _GUI_VIDEO_SCREENSAVER_OPTIONS_H_
