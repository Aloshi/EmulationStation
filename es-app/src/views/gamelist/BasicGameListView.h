#pragma once

#include "views/gamelist/ISimpleGameListView.h"
#include "components/TextListComponent.h"

class BasicGameListView : public ISimpleGameListView
{
public:
	BasicGameListView(Window* window, FileData* root);

	// Called when a FileData* is added, has its metadata changed, or is removed
	virtual void onFileChanged(FileData* file, FileChangeType change);

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	virtual FileData* getCursor() override;
	virtual int getCursorIndex() override;
	virtual void setCursor(FileData* file) override;
	virtual void setCursorIndex(int index) override;

	virtual const char* getName() const override { return "basic"; }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

	virtual void populateList(const std::vector<FileData*>& files) override;

	virtual inline void updateInfoPanel() override {}
protected:
	virtual void launch(FileData* game) override;

	TextListComponent<FileData*> mList;
};
