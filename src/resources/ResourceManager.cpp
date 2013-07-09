#include "ResourceManager.h"
#include "../Log.h"
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

auto array_deleter = [](unsigned char* p) { delete[] p; };
auto nop_deleter = [](unsigned char* p) { };


//from ES_logo_16.cpp
extern const size_t es_logo_16_data_len;
extern const unsigned char es_logo_16_data[];

//from ES_logo_32.cpp
extern const size_t es_logo_32_data_len;
extern const unsigned char es_logo_32_data[];

struct EmbeddedResource
{
	const char* internal_path;
	ResourceData resourceData;
};

static const int embedded_resource_count = 2;
static const EmbeddedResource embedded_resources[embedded_resource_count] = {
	{ "internal://es_logo_16.png", {std::shared_ptr<unsigned char>((unsigned char*)es_logo_16_data, nop_deleter), es_logo_16_data_len} }, 
	{ "internal://es_logo_32.png", {std::shared_ptr<unsigned char>((unsigned char*)es_logo_32_data, nop_deleter), es_logo_32_data_len} }
};


const ResourceData ResourceManager::getFileData(const std::string& path) const
{
	for(int i = 0; i < embedded_resource_count; i++)
	{
		if(strcmp(embedded_resources[i].internal_path, path.c_str()) == 0)
		{
			//this embedded resource matches the filepath; use it
			return embedded_resources[i].resourceData;
		}
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
	for(int i = 0; i < embedded_resource_count; i++)
	{
		if(strcmp(embedded_resources[i].internal_path, path.c_str()) == 0)
		{
			//this embedded resource matches the filepath
			return true;
		}
	}

	return fs::exists(path);
}

void ResourceManager::unloadAll()
{
	auto iter = mReloadables.begin();
	while(iter != mReloadables.end())
	{
		if(!iter->expired())
		{
			iter->lock()->unload(*this);
			iter++;
		}else{
			mReloadables.erase(iter++);
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
			iter->lock()->reload(*this);
			iter++;
		}else{
			mReloadables.erase(iter++);
		}
	}
}

void ResourceManager::addReloadable(std::weak_ptr<IReloadable> reloadable)
{
	mReloadables.push_back(reloadable);
}
