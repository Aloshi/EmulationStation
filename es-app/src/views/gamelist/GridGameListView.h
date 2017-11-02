#pragma once
#ifndef ES_APP_VIEWS_GAME_LIST_GRID_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_GRID_GAME_LIST_VIEW_H

#include "components/ImageGridComponent.h"
#include "views/gamelist/ISimpleGameListView.h"

class GridGameListView : public ISimpleGameListView
{
public:
	GridGameListView(Window* window, FileData* root);

	//virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual FileData* getCursor() override;
	virtual void setCursor(FileData*) override;

	virtual bool input(InputConfig* config, Input input) override;

	virtual const char* getName() const override { return "grid"; }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;
	virtual void launch(FileData* game) override;

protected:
	virtual void populateList(const std::vector<FileData*>& files) override;

	ImageGridComponent<FileData*> mGrid;
};

#endif // ES_APP_VIEWS_GAME_LIST_GRID_GAME_LIST_VIEW_H
