#pragma once

#include "ResourceManager.h"

#include <string>
#include <Eigen/Dense>
#include "../platform.h"
#include GLHEADER

// An OpenGL texture.
// Automatically recreates the texture with renderer deinit/reinit.
class TextureResource : public IReloadable
{
public:
	static std::shared_ptr<TextureResource> get(const std::string& path, bool tile = false);

	virtual ~TextureResource();

	virtual void unload(std::shared_ptr<ResourceManager>& rm) override;
	virtual void reload(std::shared_ptr<ResourceManager>& rm) override;
	
	bool isTiled() const;
	const Eigen::Vector2i& getSize() const;
	void bind() const;
	
	// Warning: will NOT correctly reinitialize when this texture is reloaded (e.g. ES starts/stops playing a game).
	virtual void initFromMemory(const char* file, size_t length);

	// Warning: will NOT correctly reinitialize when this texture is reloaded (e.g. ES starts/stops playing a game).
	void initFromPixels(const unsigned char* dataRGBA, size_t width, size_t height);

protected:
	TextureResource(const std::string& path, bool tile);
	void deinit();

	Eigen::Vector2i mTextureSize;
	const std::string mPath;
	const bool mTile;

private:
	GLuint mTextureID;
	typedef std::pair<std::string, bool> TextureKeyType;
	static std::map< TextureKeyType, std::weak_ptr<TextureResource> > sTextureMap;
};
