#include "resources/TextureResource.h"
#include "Log.h"
#include "platform.h"
#include GLHEADER
#include "ImageIO.h"
#include "Renderer.h"
#include "Util.h"
#include "resources/SVGResource.h"

std::map< TextureResource::TextureKeyType, std::weak_ptr<TextureResource> > TextureResource::sTextureMap;
std::list< std::weak_ptr<TextureResource> > TextureResource::sTextureList;

TextureResource::TextureResource(const std::string& path, bool tile) : 
	mTextureID(0), mPath(path), mTextureSize(Eigen::Vector2i::Zero()), mTile(tile)
{
}

TextureResource::~TextureResource()
{
	deinit();
}

void TextureResource::unload(std::shared_ptr<ResourceManager>& rm)
{
	deinit();
}

void TextureResource::reload(std::shared_ptr<ResourceManager>& rm)
{
	if(!mPath.empty())
	{
		const ResourceData& data = rm->getFileData(mPath);
		initFromMemory((const char*)data.ptr.get(), data.length);
	}
}

void TextureResource::initFromPixels(const unsigned char* dataRGBA, size_t width, size_t height)
{
	deinit();

	assert(width > 0 && height > 0);

	//now for the openGL texture stuff
	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataRGBA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	const GLint wrapMode = mTile ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	mTextureSize << width, height;
}

void TextureResource::initFromMemory(const char* data, size_t length)
{
	size_t width, height;
	std::vector<unsigned char> imageRGBA = ImageIO::loadFromMemoryRGBA32((const unsigned char*)(data), length, width, height);

	if(imageRGBA.size() == 0)
	{
		LOG(LogError) << "Could not initialize texture from memory, invalid data!  (file path: " << mPath << ", data ptr: " << (size_t)data << ", reported size: " << length << ")";
		return;
	}

	initFromPixels(imageRGBA.data(), width, height);
}

void TextureResource::deinit()
{
	if(mTextureID != 0)
	{
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
	}
}

const Eigen::Vector2i& TextureResource::getSize() const
{
	return mTextureSize;
}

bool TextureResource::isTiled() const
{
	return mTile;
}

void TextureResource::bind() const
{
	if(mTextureID != 0)
		glBindTexture(GL_TEXTURE_2D, mTextureID);
	else
		LOG(LogError) << "Tried to bind uninitialized texture!";
}


std::shared_ptr<TextureResource> TextureResource::get(const std::string& path, bool tile)
{
	std::shared_ptr<ResourceManager>& rm = ResourceManager::getInstance();

	const std::string canonicalPath = getCanonicalPath(path);

	if(canonicalPath.empty())
	{
		std::shared_ptr<TextureResource> tex(new TextureResource("", tile));
		rm->addReloadable(tex); //make sure we get properly deinitialized even though we do nothing on reinitialization
		return tex;
	}

	TextureKeyType key(canonicalPath, tile);
	auto foundTexture = sTextureMap.find(key);
	if(foundTexture != sTextureMap.end())
	{
		if(!foundTexture->second.expired())
			return foundTexture->second.lock();
	}

	// need to create it
	std::shared_ptr<TextureResource> tex;

	// is it an SVG?
	if(key.first.substr(key.first.size() - 4, std::string::npos) == ".svg")
	{
		// probably
		// don't add it to our map because 2 svgs might be rasterized at different sizes
		tex = std::shared_ptr<SVGResource>(new SVGResource(key.first, tile));
		sTextureList.push_back(tex); // add it to our list though
		rm->addReloadable(tex);
		tex->reload(rm);
		return tex;
	}else{
		// normal texture
		tex = std::shared_ptr<TextureResource>(new TextureResource(key.first, tile));
		sTextureMap[key] = std::weak_ptr<TextureResource>(tex);
		sTextureList.push_back(tex);
		rm->addReloadable(tex);
		tex->reload(ResourceManager::getInstance());
		return tex;
	}
}

bool TextureResource::isInitialized() const
{
	return mTextureID != 0;
}

size_t TextureResource::getMemUsage() const
{
	if(!mTextureID || mTextureSize.x() == 0 || mTextureSize.y() == 0)
		return 0;

	return mTextureSize.x() * mTextureSize.y() * 4;
}

size_t TextureResource::getTotalMemUsage()
{
	size_t total = 0;

	auto it = sTextureList.begin();
	while(it != sTextureList.end())
	{
		if((*it).expired())
		{
			// remove expired textures from the list
			it = sTextureList.erase(it);
			continue;
		}

		total += (*it).lock()->getMemUsage();
		it++;
	}

	return total;
}
