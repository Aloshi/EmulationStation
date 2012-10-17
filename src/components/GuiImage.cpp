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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
	if(mTextureID && getOpacity() > 0)
	{
		GLfloat points[12], texs[12];
		GLubyte colors[6*4];

		if(mTiled)
		{
			float xCount = ((float)mResizeWidth/mWidth);
			float yCount = ((float)mResizeHeight/mHeight);

			Renderer::buildGLColorArray(colors, 0xFFFFFF00 | (getOpacity()), 6);
			buildImageArray(getOffsetX(), getOffsetY(), points, texs, xCount, yCount);
		}else{
			Renderer::buildGLColorArray(colors, 0xFFFFFF00 | (getOpacity()), 6);
			buildImageArray(getOffsetX(), getOffsetY(), points, texs);
		}

		drawImageArray(points, texs, colors, 6);
	}
}

void GuiImage::buildImageArray(int posX, int posY, GLfloat* points, GLfloat* texs, float px, float py)
{
	points[0] = posX - (mDrawWidth * mOriginX) * px;		points[1] = posY - (mDrawHeight * mOriginY) * py;
	points[2] = posX - (mDrawWidth * mOriginX) * px;		points[3] = posY + (mDrawHeight * (1 - mOriginY)) * py;
	points[4] = posX + (mDrawWidth * (1 - mOriginX)) * px;		points[5] = posY - (mDrawHeight * mOriginY) * py;

	points[6] = posX + (mDrawWidth * (1 - mOriginX)) * px;		points[7] = posY - (mDrawHeight * mOriginY) * py;
	points[8] = posX - (mDrawWidth * mOriginX) * px;		points[9] = posY + (mDrawHeight * (1 - mOriginY)) * py;
	points[10] = posX + (mDrawWidth * (1 -mOriginX)) * px;		points[11] = posY + (mDrawHeight * (1 - mOriginY)) * py;



	texs[0] = 0;		texs[1] = py;
	texs[2] = 0;		texs[3] = 0;
	texs[4] = px;		texs[5] = py;

	texs[6] = px;		texs[7] = py;
	texs[8] = 0;		texs[9] = 0;
	texs[10] = px;		texs[11] = 0;

	if(mFlipX)
	{
		for(int i = 0; i < 11; i += 2)
			if(texs[i] == px)
				texs[i] = 0;
			else
				texs[i] = px;
	}
	if(mFlipY)
	{
		for(int i = 1; i < 12; i += 2)
			if(texs[i] == py)
				texs[i] = 0;
			else
				texs[i] = py;
	}
}

void GuiImage::drawImageArray(GLfloat* points, GLfloat* texs, GLubyte* colors, unsigned int numArrays)
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if(colors != NULL)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	}

	glVertexPointer(2, GL_FLOAT, 0, points);
	glTexCoordPointer(2, GL_FLOAT, 0, texs);

	glDrawArrays(GL_TRIANGLES, 0, numArrays);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(colors != NULL)
		glDisableClientState(GL_COLOR_ARRAY);

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
