#include "GuiImage.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>

unsigned int GuiImage::getWidth() { return mDrawWidth; }
unsigned int GuiImage::getHeight() { return mDrawHeight; }

GuiImage::GuiImage(int offsetX, int offsetY, std::string path, unsigned int resizeWidth, unsigned int resizeHeight, bool resizeExact)
{
	mTextureID = 0;

	setOffsetX(offsetX);
	setOffsetY(offsetY);

	//default origin is the center of image
	mOriginX = 0.5;
	mOriginY = 0.5;

	mWidth = mDrawWidth = 0;
	mHeight = mDrawHeight = 0;

	mTiled = false;

	mResizeWidth = resizeWidth;
	mResizeHeight = resizeHeight;

	mResizeExact = resizeExact;

	mFlipX = false;
	mFlipY = false;

	if(!path.empty())
		setImage(path);
}

GuiImage::~GuiImage()
{
	unloadImage();
}

void GuiImage::loadImage(std::string path)
{
	//make sure the file *exists*
	if(!boost::filesystem::exists(path))
	{
		std::cerr << "File \"" << path << "\" not found!\n";
		return;
	}

	//make sure we don't already have an image
	unloadImage();


	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* image = NULL;
	BYTE* imageData = NULL;
	unsigned int width, height;

	//detect the filetype
	format = FreeImage_GetFileType(path.c_str(), 0);
	if(format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(path.c_str());
	if(format == FIF_UNKNOWN)
	{
		std::cerr << "Error - could not detect filetype for image \"" << path << "\"!\n";
		return;
	}


	//make sure we can read this filetype first, then load it
	if(FreeImage_FIFSupportsReading(format))
	{
		image = FreeImage_Load(format, path.c_str());
	}else{
		std::cerr << "Error - file format reading not supported for image \"" << path << "\"!\n";
		return;
	}

	//make sure it loaded properly
	if(!image)
	{
		std::cerr << "Error loading image \"" << path << "\"!\n";
		return;
	}

	//convert to 32bit
	FIBITMAP* imgConv = FreeImage_ConvertTo32Bits(image);
	FreeImage_Unload(image);
	image = imgConv;

	//get a pointer to the image data as BGRA
	imageData = FreeImage_GetBits(image);
	if(!imageData)
	{
		std::cerr << "Error retriving bits from image \"" << path << "\"!\n";
		return;
	}



	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	//if width or height are zero then something is clearly wrong
	if(!width || !height)
	{
		std::cerr << "Width or height are zero for image \"" << path << "\"!\n";
		FreeImage_Unload(image);
		return;
	}


	/*
	//set width/height to powers of 2 for OpenGL
	for(unsigned int i = 0; i < 22; i++)
	{
		unsigned int pwrOf2 = pow(2, i);
		if(!widthPwrOf2 && pwrOf2 >= width)
			widthPwrOf2 = pwrOf2;
		if(!heightPwrOf2 && pwrOf2 >= height)
			heightPwrOf2 = pwrOf2;

		if(widthPwrOf2 && heightPwrOf2)
			break;
	}

	if(!widthPwrOf2 || !heightPwrOf2)
	{
		std::cerr << "Error assigning power of two for width or height of image!\n";
		FreeImage_Unload(image);
		return;
	}*/


	//convert from BGRA to RGBA
	GLubyte* imageRGBA = new GLubyte[4*width*height];
	for(unsigned int i = 0; i < width*height; i++)
	{
		imageRGBA[i*4+0] = imageData[i*4+2];
		imageRGBA[i*4+1] = imageData[i*4+1];
		imageRGBA[i*4+2] = imageData[i*4+0];
		imageRGBA[i*4+3] = imageData[i*4+3];
	}



	//now for the openGL texture stuff
	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageRGBA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	mWidth = width;
	mHeight = height;

	//free the image data
	FreeImage_Unload(image);

	//free the memory from that pointer
	delete[] imageRGBA;

	resize();
}

void GuiImage::resize()
{
	mDrawWidth = mWidth;
	mDrawHeight = mHeight;

	//(we don't resize tiled images)
	if(!mTiled)
	{
		float resizeScaleX = 0, resizeScaleY = 0;
		if(mResizeExact)
		{
			if(mResizeWidth)
				resizeScaleX = (float)mResizeWidth / mWidth;
			if(mResizeHeight)
				resizeScaleY = (float)mResizeHeight / mHeight;
		}else{
			if(mResizeWidth && mWidth > mResizeWidth)
				resizeScaleX = (float)mResizeWidth / mWidth;

			if(mResizeHeight && mHeight > mResizeHeight)
				resizeScaleY = (float)mResizeHeight / mHeight;
		}

		if(resizeScaleX && !resizeScaleY)
			resizeScaleY = resizeScaleX;
		if(resizeScaleY && !resizeScaleX)
			resizeScaleX = resizeScaleY;

		if(resizeScaleX)
			mDrawWidth *= resizeScaleX;
		if(resizeScaleY)
			mDrawHeight *= resizeScaleY;
	}
}

void GuiImage::unloadImage()
{
	if(mTextureID)
	{
		glDeleteTextures(1, &mTextureID);

		mTextureID = 0;
	}
}

void GuiImage::setImage(std::string path)
{
	if(mPath == path)
		return;

	mPath = path;

	unloadImage();
	if(!path.empty())
		loadImage(path);

}

void GuiImage::setOrigin(float originX, float originY)
{
	mOriginX = originX;
	mOriginY = originY;
}

void GuiImage::setTiling(bool tile)
{
	mTiled = tile;

	if(mTiled)
		mResizeExact = false;
}

void GuiImage::setResize(unsigned int width, unsigned int height, bool resizeExact)
{
	mResizeWidth = width;
	mResizeHeight = height;
	mResizeExact = resizeExact;
	resize();
}

void GuiImage::setFlipX(bool flip)
{
	mFlipX = flip;
}

void GuiImage::setFlipY(bool flip)
{
	mFlipY = flip;
}

void GuiImage::onRender()
{
	if(mTextureID)
	{
		if(mTiled)
		{
			unsigned int xCount = (unsigned int)((float)mResizeWidth/mWidth + 1.5);
			unsigned int yCount = (unsigned int)((float)mResizeHeight/mHeight + 1.5);

			//std::cout << "Array size: " << xCount << "x" << yCount << "\n";

			GLfloat* points = new GLfloat[xCount * yCount * 12];
			GLfloat* texs = new GLfloat[xCount * yCount * 12];
			for(unsigned int x = 0; x < xCount; x++)
			{
				for(unsigned int y = 0; y < yCount; y++)
				{
					buildImageArray(getOffsetX() + x*mDrawWidth, getOffsetY() + y*mDrawHeight, points + (12 * (x*yCount + y)), texs + (12 * (x*yCount + y)));
				}
			}
			drawImageArray(points, texs, xCount * yCount * 6);
			delete[] points;
			delete[] texs;
		}else{
			GLfloat points[12], texs[12];
			buildImageArray(getOffsetX(), getOffsetY(), points, texs);
			drawImageArray(points, texs, 6);
		}
	}
}

void GuiImage::buildImageArray(int posX, int posY, GLfloat* points, GLfloat* texs)
{
	points[0] = posX - (mDrawWidth * mOriginX);		points[1] = posY - (mDrawHeight * mOriginY);
	points[2] = posX - (mDrawWidth * mOriginX);		points[3] = posY + (mDrawHeight * (1 - mOriginY));
	points[4] = posX + (mDrawWidth * (1 - mOriginX));	points[5] = posY - (mDrawHeight * mOriginY);

	points[6] = posX + (mDrawWidth * (1 - mOriginX));	points[7] = posY - (mDrawHeight * mOriginY);
	points[8] = posX - (mDrawWidth * mOriginX);		points[9] = posY + (mDrawHeight * (1 - mOriginY));
	points[10] = posX + (mDrawWidth * (1 -mOriginX));	points[11] = posY + (mDrawHeight * (1 - mOriginY));



	texs[0] = 0;	texs[1] = 1;
	texs[2] = 0;	texs[3] = 0;
	texs[4] = 1;	texs[5] = 1;

	texs[6] = 1;	texs[7] = 1;
	texs[8] = 0;	texs[9] = 0;
	texs[10] = 1;	texs[11] = 0;

	if(mFlipX)
	{
		for(int i = 0; i < 11; i += 2)
			texs[i] = !texs[i];
	}
	if(mFlipY)
	{
		for(int i = 1; i < 12; i += 2)
			texs[i] = !texs[i];
	}
}

void GuiImage::drawImageArray(GLfloat* points, GLfloat* texs, unsigned int numArrays)
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, points);
	glTexCoordPointer(2, GL_FLOAT, 0, texs);

	glDrawArrays(GL_TRIANGLES, 0, numArrays);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void GuiImage::onInit()
{
	if(!mPath.empty())
		loadImage(mPath);
}

void GuiImage::onDeinit()
{
	unloadImage();
}

bool GuiImage::hasImage()
{
	return !mPath.empty();
}
