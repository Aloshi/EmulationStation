#include "VideoComponent.h"
#include "ThemeData.h"

VideoComponent::VideoComponent(Window* window) : MediaComponent(window)
{
}

VideoComponent::~VideoComponent()
{
}

void VideoComponent::setVideo(const std::string& path)
{
	mVideo.open(path);
	onTextureChanged();
}

Eigen::Vector2i VideoComponent::getTextureSize()
{
	return mVideo.getTextureSize();
}

bool VideoComponent::isTextureTiled()
{
	return false;
}

GLuint VideoComponent::getTexture()
{
	return mVideo.getFrameTexture();
}

void VideoComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	MediaComponent::applyTheme(theme, view, element, properties);

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "video");
	if(elem && properties & ThemeFlags::PATH && elem->has("path"))
		setVideo(elem->get<std::string>("path"));
}

void VideoComponent::update(int deltaTime)
{
	mVideo.advance((unsigned int)deltaTime);
	MediaComponent::update(deltaTime);
}
