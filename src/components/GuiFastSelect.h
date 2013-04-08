#ifndef _GUIFASTSELECT_H_
#define _GUIFASTSELECT_H_

#include "../Gui.h"
#include "../SystemData.h"
#include "../FolderData.h"
#include "../Sound.h"
#include "GuiList.h"
#include "GuiBox.h"

class GuiGameList;

class GuiFastSelect : Gui
{
public:
	GuiFastSelect(Window* window, GuiGameList* parent, GuiList<FileData*>* list, char startLetter, GuiBoxData data, int textcolor, Sound* scrollsound, Font* font);
	~GuiFastSelect();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();
private:
	static const std::string LETTERS;
	static const int SCROLLSPEED;
	static const int SCROLLDELAY;

	void setListPos();
	void scroll();
	void setLetterID(int id);

	GuiList<FileData*>* mList;

	size_t mLetterID;
	GuiGameList* mParent;

	GuiBox* mBox;
	int mTextColor;

	int mScrollTimer, mScrollOffset;
	bool mScrolling;

	Sound* mScrollSound;
	Font* mFont;
};

#endif
