#include "SVGResource.h"
#include "nanosvg/nanosvg.h"
#include "nanosvg/nanosvgrast.h"
#include "Log.h"
#include "Util.h"
#include "ImageIO.h"

#define DPI 96

SVGResource::SVGResource(const std::string& path, bool tile) : TextureResource(path, tile), mSVGImage(NULL)
{
	mLastWidth = 0;
	mLastHeight = 0;
}

SVGResource::~SVGResource()
{
	deinitSVG();
}

void SVGResource::unload(std::shared_ptr<ResourceManager>& rm)
{
	deinitSVG();
	TextureResource::unload(rm);
}

void SVGResource::initFromMemory(const char* file, size_t length)
{
	deinit();
	deinitSVG();

	// nsvgParse excepts a modifiable, null-terminated string
	char* copy = (char*)malloc(length + 1);
	assert(copy != NULL);
	memcpy(copy, file, length);
	copy[length] = '\0';

	mSVGImage = nsvgParse(copy, "px", DPI);
	free(copy);

	if(!mSVGImage)
	{
		LOG(LogError) << "Error parsing SVG image.";
		return;
	}

	if(mLastWidth && mLastHeight)
		rasterizeAt(mLastWidth, mLastHeight);
	else
		rasterizeAt((size_t)round(mSVGImage->width), (size_t)round(mSVGImage->height));
}

void SVGResource::rasterizeAt(size_t width, size_t height)
{
	if(!mSVGImage || (width == 0 && height == 0))
		return;

	if(width == 0)
	{
		// auto scale width to keep aspect
		width = (size_t)round((height / mSVGImage->height) * mSVGImage->width);
	}else if(height == 0)
	{
		// auto scale height to keep aspect
		height = (size_t)round((width / mSVGImage->width) * mSVGImage->height);
	}

	if(width != (size_t)round(mSVGImage->width) && height != (size_t)round(mSVGImage->height))
	{
		mLastWidth = width;
		mLastHeight = height;
	}

	unsigned char* imagePx = (unsigned char*)malloc(width * height * 4);
	assert(imagePx != NULL);

	NSVGrasterizer* rast = nsvgCreateRasterizer();
	nsvgRasterize(rast, mSVGImage, 0, 0, height / mSVGImage->height, imagePx, width, height, width * 4);
	nsvgDeleteRasterizer(rast);

	ImageIO::flipPixelsVert(imagePx, width, height);

	initFromPixels(imagePx, width, height);
	free(imagePx);
}

Eigen::Vector2f SVGResource::getSourceImageSize() const
{
	if(mSVGImage)
		return Eigen::Vector2f(mSVGImage->width, mSVGImage->height);

	return Eigen::Vector2f::Zero();
}

void SVGResource::deinitSVG()
{
	if(mSVGImage)
		nsvgDelete(mSVGImage);

	mSVGImage = NULL;
}
