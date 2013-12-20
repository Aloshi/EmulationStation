#include "DetailedGameListView.h"

DetailedGameListView::DetailedGameListView(Window* window, FileData* root) : 
	BasicGameListView(window, root), 
	mDescContainer(window), mDescription(window), 
	mImage(window), mInfoBackground(window), mDivider(window)
{
	mHeaderImage.setPosition(mSize.x() * 0.25f, 0);

	mInfoBackground.setPosition(0, mSize.y() * 0.5f);
	mInfoBackground.setOrigin(0, 0.5f);
	mInfoBackground.setResize(mSize.x() * 0.5f, mSize.y(), true);
	addChild(&mInfoBackground);

	mDivider.setPosition(mSize.x() * 0.5f, mSize.y() * 0.5f);
	mDivider.setOrigin(0.5f, 0.5f);
	addChild(&mDivider);

	const float padding = 0.01f;

	mList.setPosition(mSize.x() * (0.50f + padding), mList.getPosition().y());
	mList.setSize(mSize.x() * (0.50f - 2*padding), mList.getSize().y());
	mList.setCentered(false);
	mList.setCursorChangedCallback([&](TextListComponent<FileData*>::CursorState state) { updateInfoPanel(); });

	mImage.setOrigin(0.5f, 0.0f);
	mImage.setPosition(mSize.x() * 0.25f, mList.getPosition().y());
	mImage.setResize(mSize.x() * (0.50f - 2*padding), 0, false);
	addChild(&mImage);

	mDescContainer.setPosition(mSize.x() * padding, mSize.y() * 0.2f);
	mDescContainer.setSize(mSize.x() * (0.50f - 2*padding), 0);
	mDescContainer.setAutoScroll((int)(1600 + mDescContainer.getSize().x()), 0.025f);
	addChild(&mDescContainer);

	mDescription.setSize(mDescContainer.getSize().x(), 0);
	mDescContainer.addChild(&mDescription);

	updateInfoPanel();
}

void DetailedGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	mHeaderImage.setResize(mSize.x() * 0.5f, 0, true);
	BasicGameListView::onThemeChanged(theme);

	if(mHeaderImage.getPosition().y() + mHeaderImage.getSize().y() > mImage.getPosition().y())
		mHeaderImage.setResize(0, mSize.y() * 0.185f, true);

	mDescription.setFont(theme->getFont("descriptionFont"));
	mDescription.setColor(theme->getColor("descriptionColor"));
	mInfoBackground.setImage(theme->getImage("infoBackgroundImage").getTexture());
	mInfoBackground.setTiling(theme->getImage("infoBackgroundImage").tile);

	mDivider.setImage(theme->getImage("verticalDividerImage").getTexture());
	mDivider.setResize((float)mDivider.getTextureSize().x(), mSize.y(), true);
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

		mDescContainer.setPosition(mDescContainer.getPosition().x(), mImage.getPosition().y() + mImage.getSize().y() * 1.02f);
		mDescContainer.setSize(mDescContainer.getSize().x(), mSize.y() - mDescContainer.getPosition().y());
		mDescContainer.resetAutoScrollTimer();

		mDescription.setText(file->metadata.get("desc"));
	}
}

void DetailedGameListView::onFileChanged(FileData* file, FileChangeType type)
{
	BasicGameListView::onFileChanged(file, type);

	if(type == FILE_METADATA_CHANGED && file == getCursor())
		updateInfoPanel();
}