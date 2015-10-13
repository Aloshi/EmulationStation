#include "views/gamelist/DetailedGameListView.h"
#include "views/ViewController.h"
#include "Window.h"
#include "Settings.h"
#include "animations/LambdaAnimation.h"

DetailedGameListView::DetailedGameListView(Window* window, FileData* root, SystemData* system) : 
	BasicGameListView(window, root), 
	mDescContainer(window), mDescription(window), 
	mImage(window), mSystem(system), 

	mLblRating(window), mLblReleaseDate(window), mLblDeveloper(window), mLblPublisher(window), 
	mLblGenre(window), mLblPlayers(window), mLblLastPlayed(window), mLblPlayCount(window), mLblFavorite(window),

	mRating(window), mReleaseDate(window), mDeveloper(window), mPublisher(window), 
	mGenre(window), mPlayers(window), mLastPlayed(window), mPlayCount(window), mFavorite(window)
{
	//mHeaderImage.setPosition(mSize.x() * 0.25f, 0);

	const float padding = 0.01f;

	mList.setPosition(mSize.x() * (0.50f + padding), mList.getPosition().y());
	mList.setSize(mSize.x() * (0.50f - padding), mList.getSize().y());
	mList.setAlignment(TextListComponent<FileData*>::ALIGN_LEFT);
	mList.setCursorChangedCallback([&](const CursorState& state) { updateInfoPanel(); });

	// image
	mImage.setOrigin(0.5f, 0.5f);
	mImage.setPosition(mSize.x() * 0.25f, mList.getPosition().y() + mSize.y() * 0.2125f);
	mImage.setMaxSize(mSize.x() * (0.50f - 2*padding), mSize.y() * 0.4f);
	addChild(&mImage);

	// metadata labels + values
	mLblRating.setText("Rating: ");
	addChild(&mLblRating);
	addChild(&mRating);
	mLblReleaseDate.setText("Released: ");
	addChild(&mLblReleaseDate);
	addChild(&mReleaseDate);
	mLblDeveloper.setText("Developer: ");
	addChild(&mLblDeveloper);
	addChild(&mDeveloper);
	mLblPublisher.setText("Publisher: ");
	addChild(&mLblPublisher);
	addChild(&mPublisher);
	mLblGenre.setText("Genre: ");
	addChild(&mLblGenre);
	addChild(&mGenre);
	mLblPlayers.setText("Players: ");
	addChild(&mLblPlayers);
	addChild(&mPlayers);
	mLblLastPlayed.setText("Last played: ");
	addChild(&mLblLastPlayed);
	mLastPlayed.setDisplayMode(DateTimeComponent::DISP_RELATIVE_TO_NOW);
	addChild(&mLastPlayed);
	mLblPlayCount.setText("Times played: ");
	addChild(&mLblPlayCount);
	addChild(&mPlayCount);
	if (system->getHasFavorites())
	{
		mLblFavorite.setText("Favorite: ");
		addChild(&mLblFavorite);
		addChild(&mFavorite);
	}

	mDescContainer.setPosition(mSize.x() * padding, mSize.y() * 0.65f);
	mDescContainer.setSize(mSize.x() * (0.50f - 2*padding), mSize.y() - mDescContainer.getPosition().y());
	mDescContainer.setAutoScroll(true);
	addChild(&mDescContainer);

	mDescription.setFont(Font::get(FONT_SIZE_SMALL));
	mDescription.setSize(mDescContainer.getSize().x(), 0);
	mDescContainer.addChild(&mDescription);


	initMDLabels();
	initMDValues();
	updateInfoPanel();
}

void DetailedGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	BasicGameListView::onThemeChanged(theme);

	using namespace ThemeFlags;
	mImage.applyTheme(theme, getName(), "md_image", POSITION | ThemeFlags::SIZE);

	initMDLabels();
	std::vector<TextComponent*> labels = getMDLabels();

	if (mSystem->getHasFavorites())
	{
		assert(labels.size() == 9);
		const char* lblElements[9] = {
			"md_lbl_rating", "md_lbl_releasedate", "md_lbl_developer", "md_lbl_publisher",
			"md_lbl_genre", "md_lbl_players", "md_lbl_lastplayed", "md_lbl_playcount", "md_lbl_favorite"
		};

		for (unsigned int i = 0; i < labels.size(); i++)
		{
			labels[i]->applyTheme(theme, getName(), lblElements[i], ALL);
		}
	}
	else
	{
		assert(labels.size() == 8);
		const char* lblElements[8] = {
			"md_lbl_rating", "md_lbl_releasedate", "md_lbl_developer", "md_lbl_publisher", 
			"md_lbl_genre", "md_lbl_players", "md_lbl_lastplayed", "md_lbl_playcount"
		};

		for(unsigned int i = 0; i < labels.size(); i++)
		{
			labels[i]->applyTheme(theme, getName(), lblElements[i], ALL);
		}
	}

	initMDValues();
	std::vector<GuiComponent*> values = getMDValues();

	if (mSystem->getHasFavorites())
	{
		assert(values.size() == 9);
		const char* valElements[9] = {
			"md_rating", "md_releasedate", "md_developer", "md_publisher",
			"md_genre", "md_players", "md_lastplayed", "md_playcount", "md_favorite"
		};

		for (unsigned int i = 0; i < values.size(); i++)
		{
			values[i]->applyTheme(theme, getName(), valElements[i], ALL ^ ThemeFlags::TEXT);
		}
	}
	else
	{
		assert(values.size() == 8);
		const char* valElements[8] = {
			"md_rating", "md_releasedate", "md_developer", "md_publisher",
			"md_genre", "md_players", "md_lastplayed", "md_playcount"
		};

		for (unsigned int i = 0; i < values.size(); i++)
		{
			values[i]->applyTheme(theme, getName(), valElements[i], ALL ^ ThemeFlags::TEXT);
		}
	}

	mDescContainer.applyTheme(theme, getName(), "md_description", POSITION | ThemeFlags::SIZE);
	mDescription.setSize(mDescContainer.getSize().x(), 0);
	mDescription.applyTheme(theme, getName(), "md_description", ALL ^ (POSITION | ThemeFlags::SIZE | TEXT));
}

void DetailedGameListView::initMDLabels()
{
	using namespace Eigen;

	std::vector<TextComponent*> components = getMDLabels();

	const unsigned int colCount = 2;
	const unsigned int rowCount = components.size() / 2;

	Vector3f start(mSize.x() * 0.01f, mSize.y() * 0.625f, 0.0f);
	
	const float colSize = (mSize.x() * 0.48f) / colCount;
	const float rowPadding = 0.01f * mSize.y();

	for(unsigned int i = 0; i < components.size(); i++)
	{
		const unsigned int row = i % rowCount;
		Vector3f pos(0.0f, 0.0f, 0.0f);
		if(row == 0)
		{
			pos = start + Vector3f(colSize * (i / rowCount), 0, 0);
		}else{
			// work from the last component
			GuiComponent* lc = components[i-1];
			pos = lc->getPosition() + Vector3f(0, lc->getSize().y() + rowPadding, 0);
		}

		components[i]->setFont(Font::get(FONT_SIZE_SMALL));
		components[i]->setPosition(pos);
	}
}

void DetailedGameListView::initMDValues()
{
	using namespace Eigen;

	std::vector<TextComponent*> labels = getMDLabels();
	std::vector<GuiComponent*> values = getMDValues();

	std::shared_ptr<Font> defaultFont = Font::get(FONT_SIZE_SMALL);
	mRating.setSize(defaultFont->getHeight() * 5.0f, (float)defaultFont->getHeight());
	mReleaseDate.setFont(defaultFont);
	mDeveloper.setFont(defaultFont);
	mPublisher.setFont(defaultFont);
	mGenre.setFont(defaultFont);
	mPlayers.setFont(defaultFont);
	mLastPlayed.setFont(defaultFont);
	mPlayCount.setFont(defaultFont);
	mFavorite.setFont(defaultFont);

	float bottom = 0.0f;

	const float colSize = (mSize.x() * 0.48f) / 2;
	for(unsigned int i = 0; i < labels.size(); i++)
	{
		const float heightDiff = (labels[i]->getSize().y() - values[i]->getSize().y()) / 2;
		values[i]->setPosition(labels[i]->getPosition() + Vector3f(labels[i]->getSize().x(), heightDiff, 0));
		values[i]->setSize(colSize - labels[i]->getSize().x(), values[i]->getSize().y());

		float testBot = values[i]->getPosition().y() + values[i]->getSize().y();
		if(testBot > bottom)
			bottom = testBot;
	}

	mDescContainer.setPosition(mDescContainer.getPosition().x(), bottom + mSize.y() * 0.01f);
	mDescContainer.setSize(mDescContainer.getSize().x(), mSize.y() - mDescContainer.getPosition().y());
}

void DetailedGameListView::updateInfoPanel()
{
	FileData* file = (mList.size() == 0 || mList.isScrolling()) ? NULL : mList.getSelected();

	bool fadingOut;
	if(file == NULL)
	{
		//mImage.setImage("");
		//mDescription.setText("");
		fadingOut = true;
	}else{
		mImage.setImage(file->metadata.get("image"));
		mDescription.setText(file->metadata.get("desc"));
		mDescContainer.reset();

		if(file->getType() == GAME)
		{
			mRating.setValue(file->metadata.get("rating"));
			mReleaseDate.setValue(file->metadata.get("releasedate"));
			mDeveloper.setValue(file->metadata.get("developer"));
			mPublisher.setValue(file->metadata.get("publisher"));
			mGenre.setValue(file->metadata.get("genre"));
			mPlayers.setValue(file->metadata.get("players"));
			mLastPlayed.setValue(file->metadata.get("lastplayed"));
			mPlayCount.setValue(file->metadata.get("playcount"));
			mFavorite.setValue(file->metadata.get("favorite"));
		}
		
		fadingOut = false;
	}

	std::vector<GuiComponent*> comps = getMDValues();
	comps.push_back(&mImage);
	comps.push_back(&mDescription);
	std::vector<TextComponent*> labels = getMDLabels();
	comps.insert(comps.end(), labels.begin(), labels.end());

	for(auto it = comps.begin(); it != comps.end(); it++)
	{
		GuiComponent* comp = *it;
		// an animation is playing
		//   then animate if reverse != fadingOut
		// an animation is not playing
		//   then animate if opacity != our target opacity
		if((comp->isAnimationPlaying(0) && comp->isAnimationReversed(0) != fadingOut) || 
			(!comp->isAnimationPlaying(0) && comp->getOpacity() != (fadingOut ? 0 : 255)))
		{
			auto func = [comp](float t)
			{
				comp->setOpacity((unsigned char)(lerp<float>(0.0f, 1.0f, t)*255));
			};
			comp->setAnimation(new LambdaAnimation(func, 150), 0, nullptr, fadingOut);
		}
	}
}

void DetailedGameListView::launch(FileData* game)
{
	Eigen::Vector3f target(Renderer::getScreenWidth() / 2.0f, Renderer::getScreenHeight() / 2.0f, 0);
	if(mImage.hasImage())
		target << mImage.getCenter().x(), mImage.getCenter().y(), 0;

	ViewController::get()->launch(game, target);
}

std::vector<TextComponent*> DetailedGameListView::getMDLabels()
{
	std::vector<TextComponent*> ret;
	ret.push_back(&mLblRating);
	ret.push_back(&mLblReleaseDate);
	ret.push_back(&mLblDeveloper);
	ret.push_back(&mLblPublisher);
	ret.push_back(&mLblGenre);
	ret.push_back(&mLblPlayers);
	ret.push_back(&mLblLastPlayed);
	ret.push_back(&mLblPlayCount);
	if (mSystem->getHasFavorites())
	{
		ret.push_back(&mLblFavorite);
	}
	return ret;
}

std::vector<GuiComponent*> DetailedGameListView::getMDValues()
{
	std::vector<GuiComponent*> ret;
	ret.push_back(&mRating);
	ret.push_back(&mReleaseDate);
	ret.push_back(&mDeveloper);
	ret.push_back(&mPublisher);
	ret.push_back(&mGenre);
	ret.push_back(&mPlayers);
	ret.push_back(&mLastPlayed);
	ret.push_back(&mPlayCount);
	if (mSystem->getHasFavorites())
	{
		ret.push_back(&mFavorite);
	}
	return ret;
}

std::vector<HelpPrompt> DetailedGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if (Settings::getInstance()->getBool("QuickSystemSelect"))
	{
		prompts.push_back(HelpPrompt("left/right", "system"));
	}
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("y", "favorite"));
	prompts.push_back(HelpPrompt("select", "options"));
	return prompts;
}

