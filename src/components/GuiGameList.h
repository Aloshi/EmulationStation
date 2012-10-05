#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../GuiComponent.h"
#include "GuiList.h"
#include "GuiImage.h"
#include "GuiTheme.h"
#include <string>
#include <stack>
#include "../SystemData.h"
#include "../GameData.h"
#include "../FolderData.h"


//This is where the magic happens - GuiGameList is the parent of almost every graphical element in ES at the moment.
//It has a GuiList child that handles the game list, a GuiTheme that handles the theming system, and a GuiImage for game images.
class GuiGameList : GuiComponent
{
public:
	GuiGameList(bool useDetail = false);
	~GuiGameList();

	void setSystemId(int id);

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);
	void onPause();
	void onResume();
	void onInit();
	void onDeinit();

	static GuiGameList* create();

	static const float sInfoWidth;
private:
	void updateList();
	void updateTheme();
	void updateDetailData();

	SystemData* mSystem;
	FolderData* mFolder;
	std::stack<FolderData*> mFolderStack;
	int mSystemId;
	bool mDetailed;

	GuiList<FileData*>* mList;
	GuiImage* mScreenshot;
	GuiTheme* mTheme;
};

#endif
