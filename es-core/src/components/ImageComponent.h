#pragma once

#include "MediaComponent.h"
#include "resources/TextureResource.h"

class ImageComponent : public MediaComponent
{
public:
	ImageComponent(Window* window);
	virtual ~ImageComponent();

	//Loads the image at the given filepath. Will tile if tile is true (retrieves texture as tiling, creates vertices accordingly).
	void setImage(std::string path, bool tile = false);

	//Loads an image from memory.
	void setImage(const char* image, size_t length, bool tile = false);

	//Use an already existing texture.
	void setImage(const std::shared_ptr<TextureResource>& texture);

	virtual void onSizeChanged() override;
	
	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

protected:
	virtual Eigen::Vector2i getTextureSize() override;
	virtual Eigen::Vector2f getSourceTextureSize() override;
	virtual bool isTextureTiled() override;
	virtual GLuint getTexture() override;

private:
	std::shared_ptr<TextureResource> mTexture;
};
