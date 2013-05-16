#include "ImageIO.h"

#include "Log.h"


std::vector<unsigned char> ImageIO::loadFromMemoryRGBA32(const unsigned char * data, const size_t size, size_t & width, size_t & height)
{
	std::vector<unsigned char> rawData;
	width = 0;
	height = 0;
	FIMEMORY * fiMemory = FreeImage_OpenMemory((BYTE *)data, size);
	if (fiMemory != nullptr) {
		//detect the filetype from data
		FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(fiMemory);
		if (format != FIF_UNKNOWN && FreeImage_FIFSupportsReading(format))
		{
			//file type is supported. load image
			FIBITMAP * fiBitmap = FreeImage_LoadFromMemory(format, fiMemory);
			if (fiBitmap != nullptr)
			{
				//loaded. convert to 32bit
				FIBITMAP * fiConverted = FreeImage_ConvertTo32Bits(fiBitmap);
				if (fiConverted != nullptr)
				{
					width = FreeImage_GetWidth(fiConverted);
					height = FreeImage_GetHeight(fiConverted);
					unsigned int pitch = FreeImage_GetPitch(fiConverted);
					//loop through scanlines and add all pixel data to the return vector
					//this is necessary, because width*height*bpp might not be == pitch
					unsigned char * tempData = new unsigned char[width * height * 4];
					for (size_t i = 0; i < height; i++)
					{
						const BYTE * scanLine = FreeImage_GetScanLine(fiConverted, i);
						memcpy(tempData + (i * width * 4), scanLine, width * 4);
					}
					//convert from BGRA to RGBA
					for(size_t i = 0; i < width*height; i++)
					{
						RGBQUAD bgra = ((RGBQUAD *)tempData)[i];
						RGBQUAD rgba;
						rgba.rgbBlue = bgra.rgbRed;
						rgba.rgbGreen = bgra.rgbGreen;
						rgba.rgbRed = bgra.rgbBlue;
						rgba.rgbReserved = bgra.rgbReserved;
						((RGBQUAD *)tempData)[i] = rgba;
					}
					rawData = std::vector<unsigned char>(tempData, tempData + width * height * 4);
					//free converted data
					FreeImage_Unload(fiConverted);
				}
				//free bitmap data
				FreeImage_Unload(fiBitmap);
			}
		}
		else
		{
			LOG(LogError) << "Error - File type unknown/unsupported!";
		}
		//free FIMEMORY again
		FreeImage_CloseMemory(fiMemory);
	}
	return rawData;
}
