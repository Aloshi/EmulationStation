#pragma once

#include "BasicGameListView.h"
#include "../components/ImageComponent.h"
#include "../components/TextComponent.h"
#include "../components/ScrollableContainer.h"

class DetailedGameListView : public BasicGameListView
{
public:
	DetailedGameListView(Window* window, FileData* root);

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual void onFileChanged(FileData* file, FileChangeType change);

private:
	void updateInfoPanel();

	ImageComponent mImage;
	ImageComponent mInfoBackground;
	ImageComponent mDivider;

	ScrollableContainer mDescContainer;
	TextComponent mDescription;
};

