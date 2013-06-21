#include "TextureResource.h"
#include "../Log.h"
#include "../platform.h"
#include GLHEADER
#include "../ImageIO.h"
#include "../Renderer.h"

TextureResource::TextureResource() : mTextureID(0)
{
}

TextureResource::~TextureResource()
{
	deinit();
}

void TextureResource::init(ResourceData data)
{
	//make sure we aren't going to leak an old texture
	deinit();

	size_t width, height;
	std::vector<unsigned char> imageRGBA = ImageIO::loadFromMemoryRGBA32(data.ptr, data.length, width, height);

	if(imageRGBA.size() == 0)
	{
		LOG(LogError) << "Could not initialize texture!";
		return;
	}

	//now for the openGL texture stuff
	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageRGBA.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	mTextureSize.x = width;
	mTextureSize.y = height;
}

void TextureResource::deinit()
{
	if(mTextureID != 0)
	{
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
	}
}

Vector2u TextureResource::getSize()
{
	return mTextureSize;
}

void TextureResource::bind()
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
}

void TextureResource::initFromScreen()
{
	deinit();

	int width = Renderer::getScreenWidth();
	int height = Renderer::getScreenHeight();

	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, width, height, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	mTextureSize.x = height;
	mTextureSize.y = height;
}
