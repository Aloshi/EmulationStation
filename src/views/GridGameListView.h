#pragma once

#include "GameListView.h"
#include "../components/ImageGridComponent.h"
#include "../components/ImageComponent.h"
#include <stack>

class GridGameListView : public GameListView
{
public:
	GridGameListView(Window* window, FileData* root);

	virtual void onFileChanged(FileData* file, FileChangeType change) override;
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	FileData* getCursor() override;
	void setCursor(FileData*) override;

	virtual bool input(InputConfig* config, Input input) override;

private:
	void populateList(FileData* root);

	ImageGridComponent<FileData*> mGrid;
	ImageComponent mBackground;

	std::stack<FileData*> mCursorStack;
};
