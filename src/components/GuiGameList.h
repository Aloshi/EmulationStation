#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../GuiComponent.h"
#include "GuiList.h"
#include <string>

class GuiGameList : GuiComponent
{
public:
	GuiGameList(std::string systemName);

	void onRender();
private:
	std::string mSystemName;
	GuiList* mList;
};

#endif
