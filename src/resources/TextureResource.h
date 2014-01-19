#pragma once

#include "ResourceManager.h"

#include <string>
#include <Eigen/Dense>
#include "../platform.h"
#include GLHEADER

class TextureResource : public IReloadable
{
public:
	static std::shared_ptr<TextureResource> get(const std::string& path, bool tile = false);

	virtual ~TextureResource();

	void unload(std::shared_ptr<ResourceManager>& rm) override;
	void reload(std::shared_ptr<ResourceManager>& rm) override;
	
	bool isTiled() const;
	Eigen::Vector2i getSize() const;
	void bind() const;
	
	void initFromMemory(const char* image, size_t length);

private:
	TextureResource(const std::string& path, bool tile);

	void initFromPath();
	void initFromResource(const ResourceData data);
	void deinit();

	Eigen::Vector2i mTextureSize;
	GLuint mTextureID;
	const std::string mPath;
	const bool mTile;

	typedef std::pair<std::string, bool> TextureKeyType;
	static std::map< TextureKeyType, std::weak_ptr<TextureResource> > sTextureMap;
};
