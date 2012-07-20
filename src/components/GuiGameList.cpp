#include "GuiGameList.h"

GuiGameList::GuiGameList(std::string systemName)
{
	mSystemName = systemName;

	mList = new GuiList();

	addChild(mList);
}

void GuiGameList::onRender()
{
	SDL_Color color = {0, 0, 255};
	Renderer::drawCenteredText(mSystemName, 2, color);
}
