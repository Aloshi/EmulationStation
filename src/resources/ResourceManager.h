#pragma once

#include <stddef.h>
#include <memory>
#include <map>
#include <list>

//The ResourceManager exists to...
//Allow loading resources embedded into the executable like an actual file.
//Allow embedded resources to be optionally remapped to actual files for further customization.

struct ResourceData
{
	const std::shared_ptr<unsigned char> ptr;
	const size_t length;
};

class ResourceManager;

class IReloadable
{
public:
	virtual void unload(std::shared_ptr<ResourceManager>& rm) = 0;
	virtual void reload(std::shared_ptr<ResourceManager>& rm) = 0;
};

class ResourceManager
{
public:
	static std::shared_ptr<ResourceManager>& getInstance();

	void addReloadable(std::weak_ptr<IReloadable> reloadable);

	void unloadAll();
	void reloadAll();

	const ResourceData getFileData(const std::string& path) const;
	bool fileExists(const std::string& path) const;

private:
	ResourceManager();

	static std::shared_ptr<ResourceManager> sInstance;

	ResourceData loadFile(const std::string& path) const;

	std::list< std::weak_ptr<IReloadable> > mReloadables;
};
