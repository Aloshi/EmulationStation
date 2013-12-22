#pragma once

#include "../GuiComponent.h"
#include "../components/ImageComponent.h"
#include "../Log.h"

template<typename T>
class ImageGridComponent : public GuiComponent
{
public:
	ImageGridComponent(Window* window);

	struct Entry
	{
		std::shared_ptr<TextureResource> texture;
		T object;

		Entry() {}
		Entry(std::shared_ptr<TextureResource> t, const T& o) : texture(t), object(o) {}
	};

	void add(const std::string& imagePath, const T& obj);
	void remove(const T& obj);
	void clear();

	void setCursor(const T& select);
	void setCursor(typename const std::vector<Entry>::const_iterator& it);

	inline const T& getSelected() const { return mEntries.at(mCursor).object; }
	inline const std::vector<Entry>& getList() const { return mEntries; }

	enum CursorState {
		CURSOR_STOPPED,
		CURSOR_SCROLLING
	};

	void stopScrolling();

	void onSizeChanged() override;

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	Eigen::Vector2f getSquareSize(std::shared_ptr<TextureResource> tex = nullptr) const
	{
		Eigen::Vector2f aspect(1, 1);

		if(tex)
		{
			const Eigen::Vector2i& texSize = tex->getSize();

			if(texSize.x() > texSize.y())
				aspect[0] = (float)texSize.x() / texSize.y();
			else
				aspect[1] = (float)texSize.y() / texSize.x();
		}

		return Eigen::Vector2f(156 * aspect.x(), 156 * aspect.y());
	};

	Eigen::Vector2f getMaxSquareSize() const
	{
		Eigen::Vector2f squareSize(32, 32);

		// calc biggest square size
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
		{
			Eigen::Vector2f chkSize = getSquareSize(it->texture);
			if(chkSize.x() > squareSize.x())
				squareSize[0] = chkSize[0];
			if(chkSize.y() > squareSize.y())
				squareSize[1] = chkSize[1];
		}

		return squareSize;
	};

	Eigen::Vector2i getGridSize() const
	{
		Eigen::Vector2f squareSize = getMaxSquareSize();
		Eigen::Vector2i gridSize(mSize.x() / (squareSize.x() + getPadding().x()), mSize.y() / (squareSize.y() + getPadding().y()));
		return gridSize;
	};

	Eigen::Vector2f getPadding() const { return Eigen::Vector2f(24, 24); }
	
	void buildImages();
	void updateImages();

	static const int SCROLL_DELAY = 507;
	static const int SCROLL_TIME = 150;

	void setScrollDir(Eigen::Vector2i dir);
	void scroll();
	void onCursorChanged(CursorState state);

	int mCursor;

	Eigen::Vector2i mScrollDir;
	int mScrollAccumulator;

	bool mEntriesDirty;

	std::vector<Entry> mEntries;
	std::vector<ImageComponent> mImages;
};

template<typename T>
ImageGridComponent<T>::ImageGridComponent(Window* window) : GuiComponent(window)
{
	mEntriesDirty = true;
	mCursor = 0;
	mScrollDir << 0, 0;
	mScrollAccumulator = 0;
}

template<typename T>
void ImageGridComponent<T>::add(const std::string& imagePath, const T& obj)
{
	Entry e(ResourceManager::getInstance()->fileExists(imagePath) ? TextureResource::get(imagePath) : TextureResource::get(":/button.png"), obj);
	mEntries.push_back(e);
	mEntriesDirty = true;
}

template<typename T>
void ImageGridComponent<T>::remove(const T& obj)
{
	for(auto it = mEntries.begin(); it != mEntries.end(); it++)
	{
		if((*it).object == obj)
		{
			if(mCursor > 0 && it - mEntries.begin() >= mCursor)
			{
				mCursor--;
				onCursorChanged(CURSOR_STOPPED);
			}

			mEntriesDirty = true;
			mEntries.erase(it);
			return;
		}
	}

	LOG(LogError) << "Tried to remove an object we couldn't find";
}

template<typename T>
void ImageGridComponent<T>::clear()
{
	mEntries.clear();
	mCursor = 0;
	mScrollDir << 0, 0;
	onCursorChanged(CURSOR_STOPPED);
	mEntriesDirty = true;
}

template<typename T>
void ImageGridComponent<T>::setCursor(const T& obj)
{
	for(auto it = mEntries.begin(); it != mEntries.end(); it++)
	{
		if((*it).object == obj)
		{
			mCursor = it - mEntries.begin();
			onCursorChanged(CURSOR_STOPPED);
			return;
		}
	}

	LOG(LogError) << "Tried to set cursor to object we couldn't find";
}

template<typename T>
void ImageGridComponent<T>::setCursor(typename const std::vector<Entry>::const_iterator& it)
{
	assert(it != mEntries.end());
	mCursor = it - mEntries.begin();
	onCursorChanged(CURSOR_STOPPED);
}

template<typename T>
void ImageGridComponent<T>::stopScrolling()
{
	mScrollDir = Eigen::Vector2i::Zero();
}

template<typename T>
void ImageGridComponent<T>::scroll()
{
	if(mEntries.size() == 0)
		return;

	int offset = 0;
	Eigen::Vector2i size = getGridSize();
	
	offset += mScrollDir.x();
	offset += mScrollDir.y() * size.x();

	mCursor += offset;
	if(mCursor < 0)
		mCursor += mEntries.size();
	if(mCursor >= (int)mEntries.size())
		mCursor -= mEntries.size();

	onCursorChanged(CURSOR_SCROLLING);
}

template<typename T>
void ImageGridComponent<T>::setScrollDir(Eigen::Vector2i dir)
{
	mScrollDir = dir;
	mScrollAccumulator = -SCROLL_DELAY;
}

template<typename T>
bool ImageGridComponent<T>::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		Eigen::Vector2i dir = Eigen::Vector2i::Zero();
		if(config->isMappedTo("up", input))
			dir[1] = -1;
		else if(config->isMappedTo("down", input))
			dir[1] = 1;
		else if(config->isMappedTo("left", input))
			dir[0] = -1;
		else if(config->isMappedTo("right", input))
			dir[0] = 1;

		if(dir != Eigen::Vector2i::Zero())
		{
			setScrollDir(dir);
			scroll();
			return true;
		}
	}else{
		if(config->isMappedTo("up", input) || config->isMappedTo("down", input) || config->isMappedTo("left", input) || config->isMappedTo("right", input))
		{
			mScrollDir << 0, 0;
			onCursorChanged(CURSOR_STOPPED);
		}
	}

	return GuiComponent::input(config, input);
}

template<typename T>
void ImageGridComponent<T>::update(int deltaTime)
{
	if(mScrollDir != Eigen::Vector2i::Zero())
	{
		mScrollAccumulator += deltaTime;
		while(mScrollAccumulator >= SCROLL_TIME)
		{
			scroll();
			mScrollAccumulator -= SCROLL_TIME;
		}
	}
}

template<typename T>
void ImageGridComponent<T>::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = getTransform() * parentTrans;

	if(mEntriesDirty)
	{
		buildImages();
		updateImages();
		mEntriesDirty = false;
	}

	for(auto it = mImages.begin(); it != mImages.end(); it++)
	{
		it->render(trans);
	}

	renderChildren(trans);
}

template<typename T>
void ImageGridComponent<T>::onCursorChanged(CursorState state)
{
	updateImages();
}

template<typename T>
void ImageGridComponent<T>::onSizeChanged()
{
	buildImages();
	updateImages();
}

// create and position imagecomponents (mImages)
template<typename T>
void ImageGridComponent<T>::buildImages()
{
	mImages.clear();

	Eigen::Vector2i gridSize = getGridSize();
	Eigen::Vector2f squareSize = getMaxSquareSize();
	Eigen::Vector2f padding = getPadding();

	// attempt to center within our size
	Eigen::Vector2f totalSize(gridSize.x() * (squareSize.x() + padding.x()), gridSize.y() * (squareSize.y() + padding.y()));
	Eigen::Vector2f offset(mSize.x() - totalSize.x(), mSize.y() - totalSize.y());
	offset /= 2;

	for(int y = 0; y < gridSize.y(); y++)
	{
		for(int x = 0; x < gridSize.x(); x++)
		{
			mImages.push_back(ImageComponent(mWindow));
			ImageComponent& image = mImages.at(y * gridSize.x() + x);

			image.setPosition((squareSize.x() + padding.x()) * (x + 0.5f) + offset.x(), (squareSize.y() + padding.y()) * (y + 0.5f) + offset.y());
			image.setOrigin(0.5f, 0.5f);
			image.setResize(squareSize.x(), squareSize.y(), true);
			image.setImage("");
		}
	}
}

template<typename T>
void ImageGridComponent<T>::updateImages()
{
	if(mImages.empty())
		buildImages();

	Eigen::Vector2i gridSize = getGridSize();

	int cursorRow = mCursor / gridSize.x();
	int cursorCol = mCursor % gridSize.x();

	int start = (cursorRow - (gridSize.y() / 2)) * gridSize.x();

	//if we're at the end put the row as close as we can and no higher
	if(start + (gridSize.x() * gridSize.y()) >= (int)mEntries.size())
		start = gridSize.x() * ((int)mEntries.size()/gridSize.x() - gridSize.y() + 1);

	if(start < 0)
		start = 0;

	unsigned int i = (unsigned int)start;
	for(unsigned int img = 0; img < mImages.size(); img++)
	{
		ImageComponent& image = mImages.at(img);
		if(i >= mEntries.size())
		{
			image.setImage("");
			continue;
		}

		Eigen::Vector2f squareSize = getSquareSize(mEntries.at(i).texture);
		if(i == mCursor)
		{
			image.setColorShift(0xFFFFFFFF);
			image.setResize(squareSize.x() + getPadding().x() * 0.95f, squareSize.y() + getPadding().y() * 0.95f, true);
		}else{
			image.setColorShift(0xAAAAAABB);
			image.setResize(squareSize.x(), squareSize.y(), true);
		}

		image.setImage(mEntries.at(i).texture);
		i++;
	}
}
