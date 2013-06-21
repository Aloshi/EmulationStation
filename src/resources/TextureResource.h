#pragma once

#include "Resource.h"

#include <string>
#include "../Vector2.h"
#include "../platform.h"
#include GLHEADER

class TextureResource : public Resource
{
public:
	TextureResource();
	~TextureResource();

	void init(ResourceData data) override;
	void deinit() override;

	Vector2u getSize();
	void bind();

	void initFromScreen();

private:
	Vector2u mTextureSize;
	GLuint mTextureID;
};
