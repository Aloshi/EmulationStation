#include "GridTileComponent.h"

#include "resources/TextureResource.h"
#include "ThemeData.h"
#include "Renderer.h"

GridTileComponent::GridTileComponent(Window* window) : GuiComponent(window), mBackground(window)
{
	mDefaultProperties.mSize = getDefaultTileSize();
	mDefaultProperties.mPadding = Vector2f(16.0f, 16.0f);
	mDefaultProperties.mBackgroundImage = ":/frame.png";
	mDefaultProperties.mImageColor = 0xAAAAAABB;
	mDefaultProperties.mBackgroundColor = 0xAAAAEEFF;

	mSelectedProperties.mSize = getSelectedTileSize();
	mSelectedProperties.mPadding = mDefaultProperties.mPadding;
	mSelectedProperties.mBackgroundImage = mDefaultProperties.mBackgroundImage;
	mSelectedProperties.mImageColor = 0xFFFFFFFF;
	mSelectedProperties.mBackgroundColor = 0xFFFFFFFF;

	mImage = std::make_shared<ImageComponent>(mWindow);
	mImage->setOrigin(0.5f, 0.5f);

	mBackground.setOrigin(0.5f, 0.5f);

	addChild(&mBackground);
	addChild(&(*mImage));

	setSelected(false);
	setVisible(true);
}

void GridTileComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = getTransform() * parentTrans;

	if (mVisible)
		renderChildren(trans);
}

// Update all the tile properties to the new status (selected or default)
void GridTileComponent::update()
{
	const GridTileProperties& currentProperties = getCurrentProperties();

	mBackground.setImagePath(currentProperties.mBackgroundImage);

	mImage->setColorShift(currentProperties.mImageColor);
	mBackground.setCenterColor(currentProperties.mBackgroundColor);
	mBackground.setEdgeColor(currentProperties.mBackgroundColor);

	resize();
}

void GridTileComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	Vector2f screen = Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	const ThemeData::ThemeElement* elem = theme->getElement(view, "default", "gridtile");
	if (elem)
	{
		if (elem->has("size"))
			mDefaultProperties.mSize = elem->get<Vector2f>("size") * screen;

		if (elem->has("padding"))
			mDefaultProperties.mPadding = elem->get<Vector2f>("padding");

		if (elem->has("backgroundImage"))
			mDefaultProperties.mBackgroundImage = elem->get<std::string>("backgroundImage");

		if (elem->has("imageColor"))
			mDefaultProperties.mImageColor = elem->get<unsigned int>("imageColor");

		if (elem->has("backgroundColor"))
			mDefaultProperties.mBackgroundColor = elem->get<unsigned int>("backgroundColor");
	}

	elem = theme->getElement(view, "selected", "gridtile");

	mSelectedProperties.mSize = elem && elem->has("size") ?
								elem->get<Vector2f>("size") * screen :
								getSelectedTileSize();

	mSelectedProperties.mPadding = elem && elem->has("padding") ?
								   elem->get<Vector2f>("padding") :
								   mDefaultProperties.mPadding;

	mSelectedProperties.mBackgroundImage = elem && elem->has("backgroundImage") ?
										   elem->get<std::string>("backgroundImage") :
										   mDefaultProperties.mBackgroundImage;

	if (elem && elem->has("imageColor"))
		mSelectedProperties.mImageColor = elem->get<unsigned int>("imageColor");

	if (elem && elem->has("backgroundColor"))
		mSelectedProperties.mBackgroundColor = elem->get<unsigned int>("backgroundColor");
}

// Made this a static function because the ImageGridComponent need to know the default tile size
// to calculate the grid dimension before it instantiate the GridTileComponents
Vector2f GridTileComponent::getDefaultTileSize()
{
	Vector2f screen = Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	return screen * 0.22f;
}

Vector2f GridTileComponent::getSelectedTileSize() const
{
	return mDefaultProperties.mSize * 1.2f;
}

bool GridTileComponent::isSelected() const
{
	return mSelected;
}

void GridTileComponent::setImage(const std::string& path)
{
	mImage->setImage(path);
}

void GridTileComponent::setImage(const std::shared_ptr<TextureResource>& texture)
{
	mImage->setImage(texture);

	// Resize now to prevent flickering images when scrolling
	resize();
}

void GridTileComponent::setSelected(bool selected)
{
	mSelected = selected;
}

void GridTileComponent::setVisible(bool visible)
{
	mVisible = visible;
}

void GridTileComponent::resize()
{
	const GridTileProperties& currentProperties = getCurrentProperties();

	mImage->setMaxSize(currentProperties.mSize - currentProperties.mPadding);
	mBackground.fitTo(currentProperties.mSize - Vector2f(32.0f, 32.0f)); // (32f, 32f) the NinePatchComponent natural padding
	mBackground.setPosition(getSize().x() / 2, getSize().y() / 2);
}

const GridTileProperties& GridTileComponent::getCurrentProperties() const
{
	return mSelected ? mSelectedProperties : mDefaultProperties;
}