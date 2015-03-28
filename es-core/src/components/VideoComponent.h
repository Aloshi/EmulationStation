#pragma once

#include "MediaComponent.h"
#include "resources/VideoResource.h"

class VideoComponent : public MediaComponent
{
public:
	VideoComponent(Window* window);
	virtual ~VideoComponent();

	void setVideo(const std::string& path);

	virtual void update(int deltaTime) override;
	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

protected:
	virtual Eigen::Vector2i getTextureSize() override;
	virtual bool isTextureTiled() override;
	virtual GLuint getTexture() override;

private:
	VideoResource mVideo;
};
