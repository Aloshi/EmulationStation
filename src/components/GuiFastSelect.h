#ifndef _GUIFASTSELECT_H_
#define _GUIFASTSELECT_H_

#include "../GuiComponent.h"
#include "../SystemData.h"
#include "../FolderData.h"
#include "GuiList.h"
#include "GuiBox.h"

class GuiFastSelect : GuiComponent
{
public:
	GuiFastSelect(GuiComponent* parent, GuiList<FileData*>* list, char startLetter, GuiBoxData data);
	~GuiFastSelect();

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);
	void onTick(int deltaTime);
private:
	static const std::string LETTERS;
	static const int SCROLLSPEED;
	static const int SCROLLDELAY;

	void setListPos();

	void setLetterID(int id);

	GuiList<FileData*>* mList;

	size_t mLetterID;
	GuiComponent* mParent;

	GuiBox* mBox;

	int mScrollTimer, mScrollOffset;
	bool mScrolling;
};

#endif
