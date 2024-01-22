#pragma once
#ifndef ES_APP_VIEWS_GAME_LIST_IGAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_IGAME_LIST_VIEW_H

#include "renderers/Renderer.h"
#include "FileData.h"
#include "GuiComponent.h"

class ThemeData;
class Window;

// This is an interface that defines the minimum for a GameListView.
class IGameListView : public GuiComponent
{
public:
	IGameListView(Window* window, FileData* root) : GuiComponent(window), mRoot(root)
		{ setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight()); }

	virtual ~IGameListView() {}

	// Called when a new file is added, a file is removed, a file's metadata changes, or a file's children are sorted.
	// NOTE: FILE_SORTED is only reported for the topmost FileData, where the sort started.
	//       Since sorts are recursive, that FileData's children probably changed too.
	virtual void onFileChanged(FileData* file, FileChangeType change) = 0;

	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) = 0;

	void setTheme(const std::shared_ptr<ThemeData>& theme);
	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

	virtual FileData* getCursor() = 0;
	// if flag is true then the cursor position on the visible gamelist section on screen is recalculated
	// used only in list based views and only to be set true when there is no previous navigation to a game
	// see also: TextListComponent.REFRESH_LIST_CURSOR_POS for the use 'true' flag
	virtual void setCursor(FileData*, bool refreshListCursorPos = false) = 0;
	virtual int getViewportTop() = 0;
	virtual void setViewportTop(int index) = 0;

	virtual bool input(InputConfig* config, Input input) override;
	virtual void remove(FileData* game, bool deleteFile, bool refreshView=true) = 0;

	virtual const char* getName() const = 0;
	virtual void launch(FileData* game) = 0;

	virtual HelpStyle getHelpStyle() override;

	void render(const Transform4x4f& parentTrans) override;
protected:
	FileData* mRoot;
	std::shared_ptr<ThemeData> mTheme;
};

#endif // ES_APP_VIEWS_GAME_LIST_IGAME_LIST_VIEW_H
