#pragma once

#include "ResourceManager.h"

#include <string>
#include "../Eigen/Dense"
#include "../platform.h"
#include GLHEADER

class TextureResource : public IReloadable
{
public:
	static std::shared_ptr<TextureResource> get(ResourceManager& rm, const std::string& path);

	virtual ~TextureResource();

	void unload(const ResourceManager& rm) override;
	void reload(const ResourceManager& rm) override;
	
	Eigen::Vector2i getSize() const;
	void bind() const;
	
	void initFromScreen();

private:
	TextureResource(const ResourceManager& rm, const std::string& path);

	void initFromPath();
	void initFromResource(const ResourceData data);
	void deinit();

	Eigen::Vector2i mTextureSize;
	GLuint mTextureID;
	const std::string mPath;

	static std::map< std::string, std::weak_ptr<TextureResource> > sTextureMap;
};
