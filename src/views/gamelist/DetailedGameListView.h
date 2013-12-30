#pragma once

#include "BasicGameListView.h"
#include "../../components/ScrollableContainer.h"

class DetailedGameListView : public BasicGameListView
{
public:
	DetailedGameListView(Window* window, FileData* root);

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual const char* getName() const override { return "detailed"; }

protected:
	virtual void launch(FileData* game) override;

private:
	void updateInfoPanel();

	ImageComponent mImage;
	
	ScrollableContainer mDescContainer;
	TextComponent mDescription;
};
