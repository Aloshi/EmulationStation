#pragma once

#include "resources/TextureResource.h"

struct NSVGimage;

class SVGResource : public TextureResource
{
public:
	virtual ~SVGResource();

	virtual void unload(std::shared_ptr<ResourceManager>& rm) override;
	
	virtual void initFromMemory(const char* image, size_t length) override;

	void rasterizeAt(size_t width, size_t height);
	Eigen::Vector2f getSourceImageSize() const;

protected:
	friend TextureResource;
	SVGResource(const std::string& path, bool tile);
	void deinitSVG();

	NSVGimage* mSVGImage;
	size_t mLastWidth;
	size_t mLastHeight;
};
