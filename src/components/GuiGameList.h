#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../GuiComponent.h"
#include "TextListComponent.h"
#include "ImageComponent.h"
#include "ThemeComponent.h"
#include "AnimationComponent.h"
#include <string>
#include <stack>
#include "../SystemData.h"
#include "../GameData.h"
#include "../FolderData.h"

//This is where the magic happens - GuiGameList is the parent of almost every graphical element in ES at the moment.
//It has a GuiList child that handles the game list, a GuiTheme that handles the theming system, and a GuiImage for game images.
class GuiGameList : public GuiComponent
{
public:
	GuiGameList(Window* window, bool useDetail = false);
	virtual ~GuiGameList();

	void setSystemId(int id);

	bool input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	void init();
	void deinit();

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

	TextListComponent<FileData*>* mList;
	ImageComponent* mScreenshot;
	AnimationComponent* mImageAnimation;
	ThemeComponent* mTheme;
};

#endif
