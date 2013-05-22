#pragma once

#include <vector>
#include <FreeImage.h>

//This is used for loading files from Resource.h.  Just the ES logo for now (for the icon).
class ImageIO
{
public:
	static std::vector<unsigned char> loadFromMemoryRGBA32(const unsigned char * data, const size_t size, size_t & width, size_t & height);
};