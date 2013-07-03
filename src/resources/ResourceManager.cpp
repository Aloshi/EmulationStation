#include "ResourceManager.h"
#include "../Log.h"
#include <fstream>

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
	{ "internal://es_logo_16.png", {es_logo_16_data, es_logo_16_data_len} }, 
	{ "internal://es_logo_32.png", {es_logo_32_data, es_logo_32_data_len} }
};

std::shared_ptr<TextureResource> ResourceManager::getTexture(const std::string& path)
{
	if(path.empty())
		return std::shared_ptr<TextureResource>(); //NULL pointer

	if(mTextureMap[path].expired())
	{
		std::shared_ptr<TextureResource> ret(new TextureResource());
		mTextureMap[path] = std::weak_ptr<TextureResource>(ret);
		
		initializeResource(path, ret);

		return ret;
	}

	return mTextureMap[path].lock();
}

std::shared_ptr<Font> ResourceManager::getFont(const std::string& path, int size)
{
	if(path.empty() || size == 0)
		return std::shared_ptr<Font>(); //NULL pointer

	std::pair<std::string, int> fontDef(path, size);
	if(mFontMap[fontDef].expired())
	{
		std::shared_ptr<Font> ret(new Font(size));
		mFontMap[fontDef] = std::weak_ptr<Font>(ret);

		initializeResource(path, ret);

		return ret;
	}

	return mFontMap[fontDef].lock();
}

void ResourceManager::init()
{
	for(auto iter = mTextureMap.begin(); iter != mTextureMap.end(); iter++)
	{
		if(!iter->second.expired())
			initializeResource(iter->first, iter->second.lock());
	}

	for(auto iter = mFontMap.begin(); iter != mFontMap.end(); iter++)
	{
		if(!iter->second.expired())
			initializeResource(iter->first.first, iter->second.lock());
	}
}

void ResourceManager::deinit()
{
	for(auto iter = mTextureMap.begin(); iter != mTextureMap.end(); iter++)
	{
		if(!iter->second.expired())
			iter->second.lock()->deinit();
	}

	for(auto iter = mFontMap.begin(); iter != mFontMap.end(); iter++)
	{
		if(!iter->second.expired())
			iter->second.lock()->deinit();
	}
}

void ResourceManager::initializeResource(const std::string& path, std::shared_ptr<Resource> resource)
{
	for(int i = 0; i < embedded_resource_count; i++)
	{
		if(strcmp(embedded_resources[i].internal_path, path.c_str()) == 0)
		{
			//this embedded resource matches the filepath; use it
			resource->init(embedded_resources[i].resourceData);
			return;
		}
	}

	//it's not embedded; load the file, initialize with it, then free the file
	ResourceData data = loadFile(path);
	resource->init(data);
	delete[] data.ptr;
}

ResourceData ResourceManager::loadFile(const std::string& path)
{
	std::ifstream stream(path, std::ios::binary);

	stream.seekg(0, stream.end);
	size_t size = (size_t)stream.tellg();
	stream.seekg(0, stream.beg);

	unsigned char* data = new unsigned char[size];
	stream.read((char*)data, size);
	stream.close();

	ResourceData ret = {data, size};
	return ret;
}
