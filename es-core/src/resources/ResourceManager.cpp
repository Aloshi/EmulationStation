#include "ResourceManager.h"
#include "Log.h"
#include "../data/Resources.h"
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

auto array_deleter = [](unsigned char* p) { delete[] p; };
auto nop_deleter = [](unsigned char* p) { };

std::shared_ptr<ResourceManager> ResourceManager::sInstance = nullptr;

ResourceManager::ResourceManager()
{
}

std::shared_ptr<ResourceManager>& ResourceManager::getInstance()
{
	if(!sInstance)
		sInstance = std::shared_ptr<ResourceManager>(new ResourceManager());

	return sInstance;
}

const ResourceData ResourceManager::getFileData(const std::string& path) const
{
	//check if its embedded
	
	if(res2hMap.find(path) != res2hMap.end())
	{
		//it is
		Res2hEntry embeddedEntry = res2hMap.find(path)->second;
		ResourceData data = { 
			std::shared_ptr<unsigned char>(const_cast<unsigned char*>(embeddedEntry.data), nop_deleter), 
			embeddedEntry.size
		};
		return data;
	}

	//it's not embedded; load the file
	if(!fs::exists(path))
	{
		//if the file doesn't exist, return an "empty" ResourceData
		ResourceData data = {NULL, 0};
		return data;
	}else{
		ResourceData data = loadFile(path);
		return data;
	}
}

ResourceData ResourceManager::loadFile(const std::string& path) const
{
	std::ifstream stream(path, std::ios::binary);

	stream.seekg(0, stream.end);
	size_t size = (size_t)stream.tellg();
	stream.seekg(0, stream.beg);

	//supply custom deleter to properly free array
	std::shared_ptr<unsigned char> data(new unsigned char[size], array_deleter);
	stream.read((char*)data.get(), size);
	stream.close();

	ResourceData ret = {data, size};
	return ret;
}

bool ResourceManager::fileExists(const std::string& path) const
{
	//if it exists as an embedded file, return true
	if(res2hMap.find(path) != res2hMap.end())
		return true;

	return fs::exists(path);
}

void ResourceManager::unloadAll()
{
	auto iter = mReloadables.begin();
	while(iter != mReloadables.end())
	{
		if(!iter->expired())
		{
			iter->lock()->unload(sInstance);
			iter++;
		}else{
			iter = mReloadables.erase(iter);
		}
	}
}

void ResourceManager::reloadAll()
{
	auto iter = mReloadables.begin();
	while(iter != mReloadables.end())
	{
		if(!iter->expired())
		{
			iter->lock()->reload(sInstance);
			iter++;
		}else{
			iter = mReloadables.erase(iter);
		}
	}
}

void ResourceManager::addReloadable(std::weak_ptr<IReloadable> reloadable)
{
	mReloadables.push_back(reloadable);
}
