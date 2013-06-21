#pragma once

#include <stddef.h>
#include <memory>
#include <map>
#include "Resource.h"
#include "TextureResource.h"


//The ResourceManager exists to:
//1. Automatically deal with initializing/deinitializing various resources (OpenGL textures, SDL audio)
//2. Allow loading resources embedded into the executable like an actual file.
//   a. Allow embedded resources to be optionally remapped to actual files for further customization.
//3. Keep from creating duplicate resources.

//The ResourceManager returns resources as std::shared_ptrs.
//When the resource no longer has any references, it will be automatically freed.
//If ES launches a game, all resources will have deinit() called on them and SDL will deinitialize.
//Once the game exits and ES returns, resources will have init() called on them.

class ResourceManager
{
public:
	std::shared_ptr<TextureResource> getTexture(const std::string& path);

	void init();
	void deinit();

private:
	void initializeResource(const std::string& path, std::shared_ptr<Resource> resource);
	ResourceData loadFile(const std::string& path);

	std::map< std::string, std::weak_ptr<TextureResource> > mTextureMap;
};
