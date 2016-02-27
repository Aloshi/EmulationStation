#include "views/gamelist/FanartGameListView.h"
#include "views/ViewController.h"
#include "Window.h"
#include "animations/LambdaAnimation.h"

#define COL_SIZE 0.25f
#define IMAGE_SIZE 0.25f
#define LIST_SIZE 0.4f

FanartGameListView::FanartGameListView(Window* window, FileData* root) : 
	BasicGameListView(window, root), 
	mImage(window),

	mLblGenre(window), mLblPlayers(window),
	
	mGenre(window), mPlayers(window)
{
	const float padding = 0.01f;

	mList.setPosition(mSize.x() * (0.50f + padding), mList.getPosition().y());
	mList.setSize(mSize.x() * (0.50f - padding), mList.getSize().y());
	mList.setAlignment(TextListComponent<FileData*>::ALIGN_LEFT);
	mList.setCursorChangedCallback([&](const CursorState& state) { updateInfoPanel(); });

	// image
	mImage.setOrigin(0.0f, 0.0f);
	mImage.setPosition(0.0f, 0.0f);
	mImage.setMaxSize(mSize.x(), mSize.y());
	addChild(&mImage);

	// metadata labels + values
	mLblGenre.setText("Genre: ");
	addChild(&mLblGenre);
	addChild(&mGenre);
	mLblPlayers.setText("Players: ");
	addChild(&mLblPlayers);
	addChild(&mPlayers);

	initMDLabels();
	initMDValues();
	updateInfoPanel();
}

void FanartGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	mTheme = theme;
	BasicGameListView::onThemeChanged(theme);

	using namespace ThemeFlags;
	mImage.applyTheme(theme, getName(), "md_image", POSITION | ThemeFlags::SIZE);
	mFanart.applyTheme(theme, getName(), "md_fanart", POSITION | COLOR | ThemeFlags::SIZE | ThemeFlags::PATH);

	initMDLabels();
	std::vector<TextComponent*> labels = getMDLabels();
	assert(labels.size() == 2);
	const char* lblElements[2] = {
		"md_lbl_genre", "md_lbl_players"
	};

	for(unsigned int i = 0; i < labels.size(); i++)
	{
		labels[i]->applyTheme(theme, getName(), lblElements[i], ALL);
	}


	initMDValues();
	std::vector<GuiComponent*> values = getMDValues();
	assert(values.size() == 2);
	const char* valElements[2] = {
		"md_genre", "md_players"
	};

	for(unsigned int i = 0; i < values.size(); i++)
	{
		values[i]->applyTheme(theme, getName(), valElements[i], ALL ^ ThemeFlags::TEXT);
	}
}

void FanartGameListView::initMDLabels()
{
	using namespace Eigen;

	std::vector<TextComponent*> components = getMDLabels();

	Vector3f start(mSize.x() * 0.01f, mSize.y() * 0.625f, 0.0f);
	
	const float colSize = (mSize.x() * COL_SIZE);

	for(unsigned int i = 0; i < components.size(); i++)
	{
		Vector3f pos(0.0f, 0.0f, 0.0f);
		
		pos = start + Vector3f(colSize * i, 0, 0);

		components[i]->setFont(Font::get(FONT_SIZE_SMALL));
		components[i]->setPosition(pos);
	}
}

void FanartGameListView::initMDValues()
{
	using namespace Eigen;

	std::vector<TextComponent*> labels = getMDLabels();
	std::vector<GuiComponent*> values = getMDValues();

	std::shared_ptr<Font> defaultFont = Font::get(FONT_SIZE_SMALL);
	mGenre.setFont(defaultFont);
	mPlayers.setFont(defaultFont);

	float bottom = 0.0f;

	const float colSize = (mSize.x() * COL_SIZE);
	for(unsigned int i = 0; i < labels.size(); i++)
	{
		const float heightDiff = (labels[i]->getSize().y() - values[i]->getSize().y()) / 2;
		values[i]->setPosition(labels[i]->getPosition() + Vector3f(labels[i]->getSize().x(), heightDiff, 0));
		values[i]->setSize(colSize - labels[i]->getSize().x(), values[i]->getSize().y());

		float testBot = values[i]->getPosition().y() + values[i]->getSize().y();
		if(testBot > bottom)
			bottom = testBot;
	}
}

void FanartGameListView::updateInfoPanel()
{
	FileData* file = (mList.size() == 0 || mList.isScrolling()) ? NULL : mList.getSelected();

	bool fadingOut;
	if(file == NULL)
	{
		fadingOut = true;
	}else{
		mImage.setImage(file->metadata.get("image"));
		if (!file->metadata.get("fanart").empty())
			mFanart.setImage(file->metadata.get("fanart"));
		else
			if (mTheme != NULL) mFanart.applyTheme(mTheme, getName(), "md_fanart", ThemeFlags::PATH);
		if (mFanart.hasImage())
			mBackground.setOpacity(0x00);
		else
			mBackground.setOpacity(0xFF);

		if(file->getType() == GAME)
		{
			mGenre.setValue(file->metadata.get("genre"));
			mPlayers.setValue(file->metadata.get("players"));
		}
		
		fadingOut = false;
	}

	std::vector<GuiComponent*> comps = getMDValues();
	comps.push_back(&mImage);
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

void FanartGameListView::launch(FileData* game)
{
	Eigen::Vector3f target(Renderer::getScreenWidth() / 2.0f, Renderer::getScreenHeight() / 2.0f, 0);
	if(mImage.hasImage())
		target << mImage.getCenter().x(), mImage.getCenter().y(), 0;

	ViewController::get()->launch(game, target);
}

std::vector<TextComponent*> FanartGameListView::getMDLabels()
{
	std::vector<TextComponent*> ret;
	ret.push_back(&mLblGenre);
	ret.push_back(&mLblPlayers);
	return ret;
}

std::vector<GuiComponent*> FanartGameListView::getMDValues()
{
	std::vector<GuiComponent*> ret;
	ret.push_back(&mGenre);
	ret.push_back(&mPlayers);
	return ret;
}
