#pragma once
#ifndef ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H
#define ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H

#include "components/IList.h"
#include "resources/TextureResource.h"
#include "GridTileComponent.h"

enum ScrollDirection
{
	SCROLL_VERTICALLY,
	SCROLL_HORIZONTALLY
};

struct ImageGridData
{
	std::shared_ptr<TextureResource> texture;
};

template<typename T>
class ImageGridComponent : public IList<ImageGridData, T>
{
protected:
	using IList<ImageGridData, T>::mEntries;
	using IList<ImageGridData, T>::listUpdate;
	using IList<ImageGridData, T>::listInput;
	using IList<ImageGridData, T>::listRenderTitleOverlay;
	using IList<ImageGridData, T>::getTransform;
	using IList<ImageGridData, T>::mSize;
	using IList<ImageGridData, T>::mCursor;
	using IList<ImageGridData, T>::Entry;
	using IList<ImageGridData, T>::mWindow;

public:
	using IList<ImageGridData, T>::size;
	using IList<ImageGridData, T>::isScrolling;
	using IList<ImageGridData, T>::stopScrolling;

	ImageGridComponent(Window* window);

	void add(const std::string& name, const std::string& imagePath, const T& obj);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;
	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	void onSizeChanged() override;

	inline void setCursorChangedCallback(const std::function<void(CursorState state)>& func) { mCursorChangedCallback = func; }

protected:
	virtual void onCursorChanged(const CursorState& state) override;

private:
	// TILES
	void buildTiles();
	void updateTiles();
	int getStartPosition() const;
	void calcGridDimension();

	// IMAGES & ENTRIES
	bool mEntriesDirty;

	// TILES
	Vector2f mMargin;
	Vector2f mTileSize;
	Vector2i mGridDimension;
	std::shared_ptr<ThemeData> mTheme;
	std::vector< std::shared_ptr<GridTileComponent> > mTiles;

	// MISCELLANEOUS
	ScrollDirection mScrollDirection;
	std::function<void(CursorState state)> mCursorChangedCallback;
};

template<typename T>
ImageGridComponent<T>::ImageGridComponent(Window* window) : IList<ImageGridData, T>(window)
{
	Vector2f screen = Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	mEntriesDirty = true;

	mSize = screen * 0.80f;
	mMargin = screen * 0.07f;
	mTileSize = GridTileComponent::getDefaultTileSize();

	calcGridDimension();

	mScrollDirection = SCROLL_VERTICALLY;
}

template<typename T>
void ImageGridComponent<T>::add(const std::string& name, const std::string& imagePath, const T& obj)
{
	typename IList<ImageGridData, T>::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.texture = ResourceManager::getInstance()->fileExists(imagePath) ? TextureResource::get(imagePath) : TextureResource::get(":/button.png");
	static_cast<IList< ImageGridData, T >*>(this)->add(entry);
	mEntriesDirty = true;
}

template<typename T>
bool ImageGridComponent<T>::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		Vector2i dir = Vector2i::Zero();
		if(config->isMappedTo("up", input))
			dir[1 ^ mScrollDirection] = -1;
		else if(config->isMappedTo("down", input))
			dir[1 ^ mScrollDirection] = 1;
		else if(config->isMappedTo("left", input))
			dir[0 ^ mScrollDirection] = -1;
		else if(config->isMappedTo("right", input))
			dir[0 ^ mScrollDirection] = 1;

		if(dir != Vector2i::Zero())
		{
			listInput(dir.x() + dir.y() * mGridDimension.x());
			return true;
		}
	}else{
		if(config->isMappedTo("up", input) || config->isMappedTo("down", input) || config->isMappedTo("left", input) || config->isMappedTo("right", input))
		{
			stopScrolling();
		}
	}

	return GuiComponent::input(config, input);
}

template<typename T>
void ImageGridComponent<T>::update(int deltaTime)
{
	listUpdate(deltaTime);

	for(auto it = mTiles.begin(); it != mTiles.end(); it++)
		(*it)->update();
}

template<typename T>
void ImageGridComponent<T>::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = getTransform() * parentTrans;

	if(mEntriesDirty)
	{
		buildTiles();
		updateTiles();
		mEntriesDirty = false;
	}

	std::shared_ptr<GridTileComponent> selectedTile = NULL;
	for(auto it = mTiles.begin(); it != mTiles.end(); it++)
	{
		std::shared_ptr<GridTileComponent> tile = (*it);

		// If it's the selected image, keep it for later, otherwise render it now
		if(tile->isSelected())
			selectedTile = tile;
		else
			tile->render(trans);
	}

	// Render the selected image on top of the others
	if (selectedTile != NULL)
		selectedTile->render(trans);

	GuiComponent::renderChildren(trans);
}

template<typename T>
void ImageGridComponent<T>::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	// Keep the theme pointer to apply it on the tiles later on
	mTheme = theme;

	Vector2f screen = Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "imagegrid");
	if (elem)
	{
		if (elem->has("margin"))
			mMargin = elem->get<Vector2f>("margin") * screen;

		if (elem->has("scrollDirection"))
			mScrollDirection = (ScrollDirection)(elem->get<std::string>("scrollDirection") == "horizontal");
	}

	// We still need to manually get the grid tile size here,
	// so we can recalculate the new grid dimension, and THEN (re)build the tiles
	elem = theme->getElement(view, "default", "gridtile");

	mTileSize = elem && elem->has("size") ?
				elem->get<Vector2f>("size") * screen :
				GridTileComponent::getDefaultTileSize();

	// Recalculate grid dimension after theme changed
	calcGridDimension();
}

template<typename T>
void ImageGridComponent<T>::onSizeChanged()
{
	buildTiles();
	updateTiles();
}

template<typename T>
void ImageGridComponent<T>::onCursorChanged(const CursorState& state)
{
	updateTiles();

	if(mCursorChangedCallback)
		mCursorChangedCallback(state);
}

// Create and position tiles (mTiles)
template<typename T>
void ImageGridComponent<T>::buildTiles()
{
	mTiles.clear();

	Vector2f startPosition = mTileSize / 2;
	Vector2f tileDistance = mTileSize + mMargin;

	int X, Y;

	// Layout tile size and position
	for(int y = 0; y < mGridDimension.y(); y++)
	{
		for(int x = 0; x < mGridDimension.x(); x++)
		{
			// Create tiles
			auto tile = std::make_shared<GridTileComponent>(mWindow);

			// In Vertical mod, tiles are ordered from left to right, then from top to bottom
			// In Horizontal mod, tiles are ordered from top to bottom, then from left to right
			X = mScrollDirection == SCROLL_VERTICALLY ? x : y;
			Y = mScrollDirection == SCROLL_VERTICALLY ? y : x;

			tile->setPosition(X * tileDistance.x() + startPosition.x(), Y * tileDistance.y() + startPosition.y());
			tile->setOrigin(0.5f, 0.5f);
			tile->setImage("");

			if (mTheme)
				tile->applyTheme(mTheme, "grid", "gridtile", ThemeFlags::ALL);

			mTiles.push_back(tile);
		}
	}
}

template<typename T>
void ImageGridComponent<T>::updateTiles()
{
	if(mTiles.empty())
		buildTiles();

	int img = getStartPosition();

	for(int ti = 0; ti < mTiles.size(); ti++)
	{
		std::shared_ptr<GridTileComponent> tile = mTiles.at(ti);

		// If we have more tiles than we have to display images on screen, hide them
		if(img >= size())
		{
			tile->setSelected(false);
			tile->setImage("");
			tile->setVisible(false);
			continue;
		}

		tile->setSelected(img == mCursor);
		tile->setImage(mEntries.at(img).data.texture);
		tile->setVisible(true);

		img++;
	}
}

// Return the starting position (the number of the game which will be displayed on top left of the screen)
template<typename T>
int ImageGridComponent<T>::getStartPosition() const
{
	int cursorRow = mCursor / mGridDimension.x();

	int start = (cursorRow - (mGridDimension.y() / 2)) * mGridDimension.x();

	// If we are at the end put the row as close as we can and no higher, using the following formula
	// Where E is the nb of entries, X the grid x dim (nb of column), Y the grid y dim (nb of line)
	// start = first tile of last row - nb column * (nb line - 1)
	//       = (E - 1) / X * X        - X * (Y - 1)
	//       = X * ((E - 1) / X - Y + 1)
	if(start + (mGridDimension.x() * mGridDimension.y()) >= (int)mEntries.size())
		start = mGridDimension.x() * (((int)mEntries.size() - 1) / mGridDimension.x() - mGridDimension.y() + 1);

	if(start < 0)
		start = 0;

	return start;
}

// Calculate how much tiles of size mTileSize we can fit in a grid of size mSize using a margin of size mMargin
template<typename T>
void ImageGridComponent<T>::calcGridDimension()
{
	// GRID_SIZE = COLUMNS * TILE_SIZE + (COLUMNS - 1) * MARGIN
	// <=> COLUMNS = (GRID_SIZE + MARGIN) / (TILE_SIZE + MARGIN)
	Vector2f gridDimension = (mSize + mMargin) / (mTileSize + mMargin);

	mGridDimension = mScrollDirection == SCROLL_VERTICALLY ?
					 Vector2i(gridDimension.x(), gridDimension.y()) :
					 Vector2i(gridDimension.y(), gridDimension.x());
};


#endif // ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H
