#pragma once
#ifndef ES_CORE_COMPONENTS_GRID_TILE_COMPONENT_H
#define ES_CORE_COMPONENTS_GRID_TILE_COMPONENT_H

#include "NinePatchComponent.h"
#include "ImageComponent.h"

struct GridTileProperties
{
	Vector2f mSize;
	Vector2f mPadding;
	unsigned int mImageColor;
	std::string mBackgroundImage;
	Vector2f mBackgroundCornerSize;
	unsigned int mBackgroundCenterColor;
	unsigned int mBackgroundEdgeColor;
};

class GridTileComponent : public GuiComponent
{
public:
	GridTileComponent(Window* window);

	void render(const Transform4x4f& parentTrans) override;
	void update();
	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties);

	// Made this a static function because the ImageGridComponent need to know the default tile max size
	// to calculate the grid dimension before it instantiate the GridTileComponents
	static Vector2f getDefaultTileSize();
	Vector2f getSelectedTileSize() const;
	bool isSelected() const;

	void setImage(const std::string& path);
	void setImage(const std::shared_ptr<TextureResource>& texture);
	void setSelected(bool selected);
	void setVisible(bool visible);

private:
	void resize();
	const GridTileProperties& getCurrentProperties() const;

	std::shared_ptr<ImageComponent> mImage;
	NinePatchComponent mBackground;

	GridTileProperties mDefaultProperties;
	GridTileProperties mSelectedProperties;

	bool mSelected;
	bool mVisible;
};

#endif // ES_CORE_COMPONENTS_GRID_TILE_COMPONENT_H
