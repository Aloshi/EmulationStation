#include "DetailedGameListView.h"
#include "../../Window.h"
#include "../ViewController.h"

DetailedGameListView::DetailedGameListView(Window* window, FileData* root) : 
	BasicGameListView(window, root), 
	mDescContainer(window), mDescription(window), 
	mImage(window)
{
	mHeaderImage.setPosition(mSize.x() * 0.25f, 0);

	const float padding = 0.01f;

	mList.setPosition(mSize.x() * (0.50f + padding), mList.getPosition().y());
	mList.setSize(mSize.x() * (0.50f - 2*padding), mList.getSize().y());
	mList.setCentered(false);
	mList.setCursorChangedCallback([&](TextListComponent<FileData*>::CursorState state) { updateInfoPanel(); });

	mImage.setOrigin(0.5f, 0.5f);
	mImage.setPosition(mSize.x() * 0.25f, mList.getPosition().y() + mSize.y() * 0.2125f);
	mImage.setMaxSize(mSize.x() * (0.50f - 2*padding), mSize.y() * 0.425f);
	addChild(&mImage);

	mDescContainer.setPosition(mSize.x() * padding, mSize.y() * 0.65f);
	mDescContainer.setSize(mSize.x() * (0.50f - 2*padding), mSize.y() - mDescContainer.getPosition().y());
	mDescContainer.setAutoScroll((int)(1600 + mDescContainer.getSize().x()), 0.025f);
	addChild(&mDescContainer);

	mDescription.setFont(Font::get(FONT_SIZE_SMALL));
	mDescription.setSize(mDescContainer.getSize().x(), 0);
	mDescContainer.addChild(&mDescription);

	updateInfoPanel();
}

void DetailedGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	BasicGameListView::onThemeChanged(theme);

	using namespace ThemeFlags;
	mImage.applyTheme(theme, getName(), "gameimage", POSITION | ThemeFlags::SIZE);
	mDescContainer.applyTheme(theme, getName(), "infoPanel", POSITION | ThemeFlags::SIZE);
	mDescription.applyTheme(theme, getName(), "description", POSITION | ThemeFlags::SIZE | FONT_PATH | FONT_SIZE | COLOR);
}

void DetailedGameListView::updateInfoPanel()
{
	FileData* file = (mList.getList().size() == 0 || mList.isScrolling()) ? NULL : mList.getSelected();

	if(file == NULL)
	{
		mImage.setImage("");
		mDescription.setText("");
	}else{
		mImage.setImage(file->metadata.get("image"));
		
		mDescription.setText(file->metadata.get("desc"));
		mDescContainer.resetAutoScrollTimer();
	}
}

void DetailedGameListView::launch(FileData* game)
{
	Eigen::Vector3f target(Renderer::getScreenWidth() / 2.0f, Renderer::getScreenHeight() / 2.0f, 0);
	if(mImage.hasImage())
		target = mImage.getPosition();

	mWindow->getViewController()->launch(game, target);
}
