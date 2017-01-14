#pragma once

#include "views/gamelist/IGameListView.h"

#include "components/TextComponent.h"
#include "components/ImageComponent.h"

class ISimpleGameListView : public IGameListView
{
public:
	ISimpleGameListView(Window* window, const FileData& root);
	virtual ~ISimpleGameListView() {}

	virtual void onFilesChanged();
	virtual void onMetaDataChanged(const FileData& file);

	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	virtual const FileData& getCursor() = 0;
	virtual void setCursor(const FileData& file) = 0;

	virtual bool input(InputConfig* config, Input input) override;

protected:
	virtual void populateList(const std::vector<FileData>& files) = 0;
	virtual void launch(FileData& game) = 0;

	TextComponent mHeaderText;
	ImageComponent mHeaderImage;
	ImageComponent mBackground;
	
	ThemeExtras mThemeExtras;

	std::stack<FileData> mCursorStack;
};
