#pragma once
#ifndef ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H
#define ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H

#include "Log.h"
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
	std::string texturePath;
};

template<typename T>
class ImageGridComponent : public IList<ImageGridData, T>
{
protected:
	using IList<ImageGridData, T>::mEntries;
	using IList<ImageGridData, T>::mScrollTier;
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
	void updateTileAtPos(int tilePos, int imgPos, int bufferTop, int bufferBot);
	int getStartPosition() const;
	void calcGridDimension();

	// IMAGES & ENTRIES
	const int texBuffersBehind[4] = { 1, 1, 1, 1 };
	const int texBuffersForward[4] = { 1, 2, 3, 3 };
	bool mEntriesDirty;
	int mLastCursor;
	std::string mDefaultGameTexture;
	std::string mDefaultFolderTexture;

	// TILES
	bool mLastRowPartial;
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
	mLastCursor = 0;
	mDefaultGameTexture = ":/cartridge.svg";
	mDefaultFolderTexture = ":/folder.svg";

	mSize = screen * 0.80f;
	mMargin = screen * 0.07f;
	mTileSize = GridTileComponent::getDefaultTileSize();

	mScrollDirection = SCROLL_VERTICALLY;
}

template<typename T>
void ImageGridComponent<T>::add(const std::string& name, const std::string& imagePath, const T& obj)
{
	typename IList<ImageGridData, T>::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.texturePath = imagePath;

	static_cast<IList< ImageGridData, T >*>(this)->add(entry);
	mEntriesDirty = true;
}

template<typename T>
bool ImageGridComponent<T>::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		Vector2i dir = Vector2i::Zero();
		if(config->isMappedLike("up", input))
			dir[1 ^ mScrollDirection] = -1;
		else if(config->isMappedLike("down", input))
			dir[1 ^ mScrollDirection] = 1;
		else if(config->isMappedLike("left", input))
			dir[0 ^ mScrollDirection] = -1;
		else if(config->isMappedLike("right", input))
			dir[0 ^ mScrollDirection] = 1;

		if(dir != Vector2i::Zero())
		{
			listInput(dir.x() + dir.y() * mGridDimension.x());
			return true;
		}
	}else{
		if(config->isMappedLike("up", input) || config->isMappedLike("down", input) || config->isMappedLike("left", input) || config->isMappedLike("right", input))
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
		updateTiles();
		mEntriesDirty = false;
	}

	// Create a clipRect to hide tiles used to buffer texture loading
	float scaleX = trans.r0().x();
	float scaleY = trans.r1().y();

	Vector2i pos((int)Math::round(trans.translation()[0]), (int)Math::round(trans.translation()[1]));
	Vector2i size((int)Math::round(mSize.x() * scaleX), (int)Math::round(mSize.y() * scaleY));

	Renderer::pushClipRect(pos, size);

	// Render all the tiles but the selected one
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

	Renderer::popClipRect();

	// Render the selected image on top of the others
	if (selectedTile != NULL)
		selectedTile->render(trans);

	listRenderTitleOverlay(trans);

	GuiComponent::renderChildren(trans);
}

template<typename T>
void ImageGridComponent<T>::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	// Apply theme to GuiComponent but not size property, which will be applied at the end of this function
	GuiComponent::applyTheme(theme, view, element, properties ^ ThemeFlags::SIZE);

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

		if (elem->has("gameImage"))
		{
			std::string path = elem->get<std::string>("gameImage");

			if (!ResourceManager::getInstance()->fileExists(path))
				LOG(LogWarning) << "Could not replace default game image, check path: " << path;
			else
			{
				std::string oldDefaultGameTexture = mDefaultGameTexture;
				mDefaultGameTexture = path;

				// mEntries are already loaded at this point,
				// so we need to update them with new game image texture
				for (auto it = mEntries.begin(); it != mEntries.end(); it++)
				{
					if ((*it).data.texturePath == oldDefaultGameTexture)
						(*it).data.texturePath = mDefaultGameTexture;
				}
			}
		}

		if (elem->has("folderImage"))
		{
			std::string path = elem->get<std::string>("folderImage");

			if (!ResourceManager::getInstance()->fileExists(path))
				LOG(LogWarning) << "Could not replace default folder image, check path: " << path;
			else
			{
				std::string oldDefaultFolderTexture = mDefaultFolderTexture;
				mDefaultFolderTexture = path;

				// mEntries are already loaded at this point,
				// so we need to update them with new folder image texture
				for (auto it = mEntries.begin(); it != mEntries.end(); it++)
				{
					if ((*it).data.texturePath == oldDefaultFolderTexture)
						(*it).data.texturePath = mDefaultFolderTexture;
				}
			}
		}
	}

	// We still need to manually get the grid tile size here,
	// so we can recalculate the new grid dimension, and THEN (re)build the tiles
	elem = theme->getElement(view, "default", "gridtile");

	mTileSize = elem && elem->has("size") ?
				elem->get<Vector2f>("size") * screen :
				GridTileComponent::getDefaultTileSize();

	// Apply size property, will trigger a call to onSizeChanged() which will build the tiles
	GuiComponent::applyTheme(theme, view, element, ThemeFlags::SIZE);

	// Trigger the call manually if the theme have no "imagegrid" element
	if (!elem)
		buildTiles();
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

	calcGridDimension();

	Vector2f tileDistance = mTileSize + mMargin;
	Vector2f bufferSize = Vector2f(mScrollDirection == SCROLL_HORIZONTALLY ? tileDistance.x() * texBuffersForward[3] : 0,
								   mScrollDirection == SCROLL_VERTICALLY ? tileDistance.y() * texBuffersForward[3] : 0);
	Vector2f startPosition = mTileSize / 2 - bufferSize;

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
	if (!mTiles.size())
		return;

	// Stop updating the tiles at highest scroll speed
	if (mScrollTier == 3)
	{
		for (int ti = 0; ti < (int)mTiles.size(); ti++)
		{
			std::shared_ptr<GridTileComponent> tile = mTiles.at(ti);

			tile->setSelected(false);
			tile->setImage(mDefaultGameTexture);
			tile->setVisible(false);
		}
		return;
	}

	// 1 if scrolling down, -1 if scrolling up
	int scrollDirection = mCursor >= mLastCursor ? 1 : -1;

	// If going down, update from top to bottom
	// If going up, update from bottom to top
	int ti = scrollDirection == 1 ? 0 : (int)mTiles.size() - 1;
	int end = scrollDirection == 1 ? (int)mTiles.size() : -1;

	int img = getStartPosition();
	if (scrollDirection == -1)
		img += (int)mTiles.size() - 1;

	// Calculate buffer size depending on scroll speed and direction
	int bufferBehind = (texBuffersForward[3] - texBuffersBehind[mScrollTier]) * mGridDimension.x();
	int bufferForward = (texBuffersForward[3] - texBuffersForward[mScrollTier]) * mGridDimension.x();

	int bufferTop = scrollDirection == 1 ? bufferBehind : bufferForward;
	int bufferBot = scrollDirection == 1 ? bufferForward : bufferBehind;

	// Update the tiles
	while (ti != end)
	{
		updateTileAtPos(ti, img, bufferTop, bufferBot);

		ti += scrollDirection;
		img += scrollDirection;
	}

	mLastCursor = mCursor;
}

template<typename T>
void ImageGridComponent<T>::updateTileAtPos(int tilePos, int imgPos, int bufferTop, int bufferBot)
{
	std::shared_ptr<GridTileComponent> tile = mTiles.at(tilePos);

	// If we have more tiles than we have to display images on screen, hide them
	if(imgPos < 0 || imgPos >= size()
	   || tilePos < bufferTop || tilePos >= (int)mTiles.size() - bufferBot) // Same for tiles out of the buffer
	{
		tile->setSelected(false);
		tile->setImage("");
		tile->setVisible(false);
	}
	else
	{
		tile->setSelected(imgPos == mCursor);
		tile->setVisible(true);

		std::string imagePath = mEntries.at(imgPos).data.texturePath;
		if (ResourceManager::getInstance()->fileExists(imagePath))
		{
			tile->setImage(imagePath);
		}
		else
		{
			// FileType::FOLDER = 2, but FileData is our template parameter T,
			// so we don't want to bring that dependence to FileData here
			if (mEntries.at(imgPos).object->getType() == 2)
				tile->setImage(mDefaultFolderTexture);
			else
				tile->setImage(mDefaultGameTexture);
		}
	}
}

// Return the starting position (the number of the game which will be displayed on top left of the screen)
template<typename T>
int ImageGridComponent<T>::getStartPosition() const
{
	// The "partialRow" variable exist because we want to keep the same positioning behavior in both
	// case, whenever we have an integer number of rows or not (the last partial row is ignored when
	// calculating position and the cursor shouldn't end up in this row when close to the end)
	int partialRow = (int)mLastRowPartial;

	int cursorRow = mCursor / mGridDimension.x();

	int start = (cursorRow - ((mGridDimension.y() - partialRow) / 2)) * mGridDimension.x();

	// Number of tiles which are just used as a buffer for texture loading
	int bufferSize = texBuffersForward[3] * mGridDimension.x();

	if(start + (mGridDimension.x() * (mGridDimension.y() - partialRow)) >= (int)mEntries.size() + bufferSize)
	{
		// If we are at the end put the row as close as we can and no higher, using the following formula
		// Where E is the nb of entries, X the grid x dim (nb of column), Y the grid y dim (nb of line)
		// start = first tile of last row - nb column * (nb line - 1)
		//       = (E - 1) / X * X        - X * (Y - 1)
		//       = X * ((E - 1) / X - Y + 1)
		start = mGridDimension.x() * (((int)mEntries.size() - 1) / mGridDimension.x() - mGridDimension.y() + 1 + partialRow) + bufferSize;
	}

	if(start < -bufferSize)
	{
		start = -bufferSize;
	}

	return start;
}

// Calculate how much tiles of size mTileSize we can fit in a grid of size mSize using a margin of size mMargin
template<typename T>
void ImageGridComponent<T>::calcGridDimension()
{
	// GRID_SIZE = COLUMNS * TILE_SIZE + (COLUMNS - 1) * MARGIN
	// <=> COLUMNS = (GRID_SIZE + MARGIN) / (TILE_SIZE + MARGIN)
	Vector2f gridDimension = (mSize + mMargin) / (mTileSize + mMargin);

	mLastRowPartial = Math::floorf(gridDimension.y()) != gridDimension.y();

	// Ceil y dim so we can display partial last row
	mGridDimension = Vector2i(gridDimension.x(), Math::ceilf(gridDimension.y()));

	// Invert dimensions for horizontally scrolling grid
	if (mScrollDirection == SCROLL_HORIZONTALLY)
		mGridDimension = Vector2i(mGridDimension.y(), mGridDimension.x());

	// Grid dimension validation
	if (mGridDimension.x() < 1)
		LOG(LogError) << "Theme defined grid X dimension below 1";
	if (mGridDimension.y() < 1)
		LOG(LogError) << "Theme defined grid Y dimension below 1";

	// Add extra tiles to both side depending on max texture buffer
	mGridDimension.y() += texBuffersForward[3] * 2;
};


#endif // ES_CORE_COMPONENTS_IMAGE_GRID_COMPONENT_H
