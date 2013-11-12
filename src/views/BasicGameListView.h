#pragma once

#include "GameListView.h"
#include "../components/TextListComponent.h"
#include "../components/TextComponent.h"
#include "../components/ImageComponent.h"

class BasicGameListView : public GameListView
{
public:
	BasicGameListView(Window* window, FileData* root);

	// Called when a FileData* is added, has its metadata changed, or is removed
	virtual void onFileChanged(FileData* file, FileChangeType change);

	virtual bool input(InputConfig* config, Input input) override;

	virtual void setTheme(const std::shared_ptr<ThemeData>& theme) override;

	inline FileData* getCursor() { return mList.getSelected(); }
	virtual void setCursor(FileData* file) override;

protected:
	void populateList(FileData* root);
	
	TextComponent mHeaderText;
	ImageComponent mHeaderImage;
	ImageComponent mBackground;
	TextListComponent<FileData*> mList;

	inline FileData* getCurrentFolder() { return getCursor()->getParent(); }

	std::stack<FileData*> mCursorStack;
};
