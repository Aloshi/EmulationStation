#pragma once

#include "views/gamelist/ISimpleGameListView.h"
#include "components/TextListComponent.h"

class BasicGameListView : public ISimpleGameListView
{
public:
	BasicGameListView(Window* window, const FileData& root);

	// Called when a FileData* is added, has its metadata changed, or is removed
	virtual void onMetaDataChanged(const FileData& file) override;

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	virtual const FileData& getCursor() override;
	virtual void setCursor(const FileData& file) override;

	virtual const char* getName() const override { return "basic"; }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

protected:
	virtual void populateList(const std::vector<FileData>& files) override;
	virtual void launch(FileData& game) override;

	TextListComponent<FileData> mList;
};
