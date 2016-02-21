#pragma once

#include "views/gamelist/BasicGameListView.h"
#include "components/ScrollableContainer.h"
#include "components/RatingComponent.h"
#include "components/DateTimeComponent.h"

class FanartGameListView : public BasicGameListView
{
public:
	FanartGameListView(Window* window, FileData* root);

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

	virtual const char* getName() const override { return "fanart"; }

protected:
	virtual void launch(FileData* game) override;

private:
	void updateInfoPanel();

	void initMDLabels();
	void initMDValues();

	ImageComponent mImage;

	TextComponent mLblGenre, mLblPlayers;

	TextComponent mGenre;
	TextComponent mPlayers;

	std::vector<TextComponent*> getMDLabels();
	std::vector<GuiComponent*> getMDValues();
};
