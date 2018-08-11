#pragma once
#ifndef ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H

#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "views/gamelist/IGameListView.h"
#include <stack>

class ISimpleGameListView : public IGameListView
{
public:
	ISimpleGameListView(Window* window, FileData* root);
	virtual ~ISimpleGameListView() {}

	// Called when a new file is added, a file is removed, a file's metadata changes, or a file's children are sorted.
	// NOTE: FILE_SORTED is only reported for the topmost FileData, where the sort started.
	//       Since sorts are recursive, that FileData's children probably changed too.
	virtual void onFileChanged(FileData* file, FileChangeType change);
	
	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	virtual FileData* getCursor() = 0;
	virtual void setCursor(FileData*) = 0;

	virtual bool input(InputConfig* config, Input input) override;
	virtual void launch(FileData* game) = 0;

protected:
	virtual std::string getQuickSystemSelectRightButton() = 0;
	virtual std::string getQuickSystemSelectLeftButton() = 0;
	virtual void populateList(const std::vector<FileData*>& files) = 0;

	TextComponent mHeaderText;
	ImageComponent mHeaderImage;
	ImageComponent mBackground;

	std::vector<GuiComponent*> mThemeExtras;

	std::stack<FileData*> mCursorStack;
};

#endif // ES_APP_VIEWS_GAME_LIST_ISIMPLE_GAME_LIST_VIEW_H
