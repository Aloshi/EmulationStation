#pragma once

#include "ResourceManager.h"

#include <string>
#include "../Vector2.h"
#include "../platform.h"
#include GLHEADER

class TextureResource : public IReloadable
{
public:
	static std::shared_ptr<TextureResource> get(ResourceManager& rm, const std::string& path);

	virtual ~TextureResource();

	void unload(const ResourceManager& rm) override;
	void reload(const ResourceManager& rm) override;
	
	Vector2u getSize() const;
	void bind() const;
	
	void initFromScreen();

private:
	TextureResource(const ResourceManager& rm, const std::string& path);

	void initFromPath();
	void initFromResource(const ResourceData data);
	void deinit();

	Vector2u mTextureSize;
	GLuint mTextureID;
	const std::string mPath;

	static std::map< std::string, std::weak_ptr<TextureResource> > sTextureMap;
};
