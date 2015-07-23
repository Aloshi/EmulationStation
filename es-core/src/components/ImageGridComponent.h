#pragma once

#include "GuiComponent.h"
#include "components/IList.h"
#include "components/ImageComponent.h"
#include "Log.h"

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
    using typename IList<ImageGridData, T>::Entry;
	using IList<ImageGridData, T>::mWindow;

public:
	using IList<ImageGridData, T>::size;
	using IList<ImageGridData, T>::isScrolling;
	using IList<ImageGridData, T>::stopScrolling;

	ImageGridComponent(Window* window);

	void add(const std::string& name, const std::string& imagePath, const T& obj);
	
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
			Eigen::Vector2f chkSize = getSquareSize(it->data.texture);
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

	virtual void onCursorChanged(const CursorState& state);

	bool mEntriesDirty;

	std::vector<ImageComponent> mImages;
};

template<typename T>
ImageGridComponent<T>::ImageGridComponent(Window* window) : IList<ImageGridData, T>(window)
{
	mEntriesDirty = true;
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
			listInput(dir.x() + dir.y() * getGridSize().x());
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

	GuiComponent::renderChildren(trans);
}

template<typename T>
void ImageGridComponent<T>::onCursorChanged(const CursorState& state)
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
			image.setResize(squareSize.x(), squareSize.y());
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
		if(i >= (unsigned int)size())
		{
			image.setImage("");
			continue;
		}

		Eigen::Vector2f squareSize = getSquareSize(mEntries.at(i).data.texture);
		if(i == mCursor)
		{
			image.setColorShift(0xFFFFFFFF);
			image.setResize(squareSize.x() + getPadding().x() * 0.95f, squareSize.y() + getPadding().y() * 0.95f);
		}else{
			image.setColorShift(0xAAAAAABB);
			image.setResize(squareSize.x(), squareSize.y());
		}

		image.setImage(mEntries.at(i).data.texture);
		i++;
	}
}
