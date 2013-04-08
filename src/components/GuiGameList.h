#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../Gui.h"
#include "GuiList.h"
#include "GuiImage.h"
#include "GuiTheme.h"
#include "GuiAnimation.h"
#include <string>
#include <stack>
#include "../SystemData.h"
#include "../GameData.h"
#include "../FolderData.h"

//This is where the magic happens - GuiGameList is the parent of almost every graphical element in ES at the moment.
//It has a GuiList child that handles the game list, a GuiTheme that handles the theming system, and a GuiImage for game images.
class GuiGameList : public Gui
{
public:
	GuiGameList(Window* window, bool useDetail = false);
	~GuiGameList();

	void setSystemId(int id);

	void render();
	void input(InputConfig* config, Input input);

	void onInit();
	void onDeinit();

	void updateDetailData();

	static GuiGameList* create(Window* window);

	static const float sInfoWidth;
private:
	void updateList();
	void updateTheme();
	void clearDetailData();
	std::string getThemeFile();

	SystemData* mSystem;
	FolderData* mFolder;
	std::stack<FolderData*> mFolderStack;
	int mSystemId;
	bool mDetailed;

	GuiList<FileData*>* mList;
	GuiImage* mScreenshot;
	GuiAnimation* mImageAnimation;
	GuiTheme* mTheme;
};

#endif
