#pragma once

#include "BasicGameListView.h"
#include "../../components/ScrollableContainer.h"
#include "../../components/RatingComponent.h"
#include "../../components/DateTimeComponent.h"

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

	void initMDLabels();
	void initMDValues();

	ImageComponent mImage;

	TextComponent mLblRating, mLblReleaseDate, mLblLastPlayed, mLblPlayCount;

	RatingComponent mRating;
	DateTimeComponent mReleaseDate;
	DateTimeComponent mLastPlayed;
	TextComponent mPlayCount;

	std::vector<TextComponent*> getMDLabels();
	std::vector<GuiComponent*> getMDValues();

	ScrollableContainer mDescContainer;
	TextComponent mDescription;
};
