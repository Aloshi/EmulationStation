#include "components/ImageComponent.h"
#include <boost/filesystem.hpp>
#include <math.h>
#include "Log.h"
#include "ThemeData.h"
#include "Util.h"
#include "resources/SVGResource.h"

ImageComponent::ImageComponent(Window* window) : MediaComponent(window)
{
}

ImageComponent::~ImageComponent()
{
}

void ImageComponent::setImage(std::string path, bool tile)
{
	if(path.empty() || !ResourceManager::getInstance()->fileExists(path))
		mTexture.reset();
	else
		mTexture = TextureResource::get(path, tile);

	onTextureChanged();
}

void ImageComponent::setImage(const char* path, size_t length, bool tile)
{
	mTexture.reset();

	mTexture = TextureResource::get("", tile);
	mTexture->initFromMemory(path, length);

	onTextureChanged();
}

void ImageComponent::setImage(const std::shared_ptr<TextureResource>& texture)
{
	mTexture = texture;
	onTextureChanged();
}

void ImageComponent::onSizeChanged()
{
	SVGResource* svg = dynamic_cast<SVGResource*>(mTexture.get());
	if(svg)
		svg->rasterizeAt((int)round(mSize.x()), (int)round(mSize.y()));

	MediaComponent::onSizeChanged();
}

bool ImageComponent::isTextureTiled()
{
	return mTexture ? mTexture->isTiled() : false;
}

Eigen::Vector2f ImageComponent::getSourceTextureSize()
{
	if(!mTexture)
		return Eigen::Vector2f::Zero();

	SVGResource* svg = dynamic_cast<SVGResource*>(mTexture.get());
	if(svg)
		return svg->getSourceImageSize();
	else
		return  mTexture->getSize().cast<float>();
}

Eigen::Vector2i ImageComponent::getTextureSize()
{
	return  mTexture->getSize();
}

GLuint ImageComponent::getTexture()
{
	return mTexture ? mTexture->getTextureId() : 0;
}

void ImageComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	MediaComponent::applyTheme(theme, view, element, properties);

	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "image");
	if(!elem)
		return;

	if(properties & PATH && elem->has("path"))
	{
		bool tile = (elem->has("tile") && elem->get<bool>("tile"));
		setImage(elem->get<std::string>("path"), tile);
	}
}