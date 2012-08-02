#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../GuiComponent.h"
#include "GuiList.h"
#include "GuiImage.h"
#include <string>
#include <stack>
#include "../SystemData.h"
#include "../GameData.h"
#include "../FolderData.h"

class GuiGameList : GuiComponent
{
public:
	GuiGameList(bool useDetail = false);
	~GuiGameList();

	void updateList();
	void setSystemId(int id);

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);
	void onPause();
	void onResume();

private:
	SystemData* mSystem;
	FolderData* mFolder;
	std::stack<FolderData*> mFolderStack;
	int mSystemId;
	bool mDetailed;

	GuiList* mList;
	GuiImage* mScreenshot;
};

#endif
