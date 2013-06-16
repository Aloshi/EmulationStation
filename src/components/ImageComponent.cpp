#include "ImageComponent.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>
#include "../Log.h"
#include "../Renderer.h"

Vector2u ImageComponent::getTextureSize() { return mTextureSize; }

ImageComponent::ImageComponent(Window* window, int offsetX, int offsetY, std::string path, unsigned int resizeWidth, unsigned int resizeHeight, bool allowUpscale) : GuiComponent(window)
{
	mTextureID = 0;

	setOffset(Vector2i(offsetX, offsetY));

	//default origin is the center of image
	mOrigin.x = 0.5;
	mOrigin.y = 0.5;

	mOpacity = 255;

	mTiled = false;

	mTargetSize.x = resizeWidth;
	mTargetSize.y = resizeHeight;

	mAllowUpscale = allowUpscale;

	mFlipX = false;
	mFlipY = false;

	if(!path.empty())
		setImage(path);
}

ImageComponent::~ImageComponent()
{
	unloadImage();
}

void ImageComponent::loadImage(std::string path)
{
	//make sure the file *exists*
	if(!boost::filesystem::exists(path))
	{
		LOG(LogError) << "Image \"" << path << "\" not found!";
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
		LOG(LogError) << "Error - could not detect filetype for image \"" << path << "\"!";
		return;
	}


	//make sure we can read this filetype first, then load it
	if(FreeImage_FIFSupportsReading(format))
	{
		image = FreeImage_Load(format, path.c_str());
	}else{
		LOG(LogError) << "Error - file format reading not supported for image \"" << path << "\"!";
		return;
	}

	//make sure it loaded properly
	if(!image)
	{
		LOG(LogError) << "Error loading image \"" << path << "\"!";
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
		LOG(LogError) << "Error retriving bits from image \"" << path << "\"!";
		return;
	}



	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	//if width or height are zero then something is clearly wrong
	if(!width || !height)
	{
		LOG(LogError) << "Width or height are zero for image \"" << path << "\"!";
		FreeImage_Unload(image);
		return;
	}

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	mTextureSize.x = width;
	mTextureSize.y = height;

	//free the image data
	FreeImage_Unload(image);

	//free the memory from that pointer
	delete[] imageRGBA;

	resize();
}

void ImageComponent::resize()
{
	mSize.x = mTextureSize.x;
	mSize.y = mTextureSize.y;

	//(we don't resize tiled images)
	if(!mTiled && (mTargetSize.x || mTargetSize.y))
	{
		Vector2f resizeScale;

		if(mTargetSize.x && (mAllowUpscale || mSize.x > mTargetSize.x))
		{
			resizeScale.x = (float)mTargetSize.x / mSize.x;
		}
		if(mTargetSize.y && (mAllowUpscale || mSize.y > mTargetSize.y))
		{
			resizeScale.y = (float)mTargetSize.y / mSize.y;
		}

		if(resizeScale.x && !resizeScale.y)
			resizeScale.y = resizeScale.x;
		if(resizeScale.y && !resizeScale.x)
			resizeScale.x = resizeScale.y;

		if(resizeScale.x)
			mSize.x = (int)(mSize.x * resizeScale.x);
		if(resizeScale.y)
			mSize.y = (int)(mSize.y * resizeScale.y);
	}

	if(mTiled)
	{
		mSize = mTargetSize;
	}
}

void ImageComponent::unloadImage()
{
	if(mTextureID)
	{
		glDeleteTextures(1, &mTextureID);

		mTextureID = 0;
	}
}

void ImageComponent::setImage(std::string path)
{
	if(mPath == path)
		return;

	mPath = path;

	unloadImage();
	if(!path.empty())
		loadImage(path);

}

void ImageComponent::setOrigin(float originX, float originY)
{
	mOrigin.x = originX;
	mOrigin.y = originY;
}

void ImageComponent::setTiling(bool tile)
{
	mTiled = tile;

	if(mTiled)
		mAllowUpscale = false;

	resize();
}

void ImageComponent::setResize(unsigned int width, unsigned int height, bool allowUpscale)
{
	mTargetSize.x = width;
	mTargetSize.y = height;
	mAllowUpscale = allowUpscale;
	resize();
}

void ImageComponent::setFlipX(bool flip)
{
	mFlipX = flip;
}

void ImageComponent::setFlipY(bool flip)
{
	mFlipY = flip;
}

void ImageComponent::onRender()
{
	if(mTextureID && getOpacity() > 0)
	{
		GLfloat points[12], texs[12];
		GLubyte colors[6*4];

		if(mTiled)
		{
			float xCount = (float)mSize.x / mTextureSize.x;
			float yCount = (float)mSize.y / mTextureSize.y;
			
			Renderer::buildGLColorArray(colors, 0xFFFFFF00 | (getOpacity()), 6);
			buildImageArray(0, 0, points, texs, xCount, yCount);
		}else{
			Renderer::buildGLColorArray(colors, 0xFFFFFF00 | (getOpacity()), 6);
			buildImageArray(0, 0, points, texs);
		}

		drawImageArray(points, texs, colors, 6);
	}

	GuiComponent::onRender();
}

void ImageComponent::buildImageArray(int posX, int posY, GLfloat* points, GLfloat* texs, float px, float py)
{
	points[0] = posX - (mSize.x * mOrigin.x);		points[1] = posY - (mSize.y * mOrigin.y);
	points[2] = posX - (mSize.x * mOrigin.x);		points[3] = posY + (mSize.y * (1 - mOrigin.y));
	points[4] = posX + (mSize.x * (1 - mOrigin.x));		points[5] = posY - (mSize.y * mOrigin.y);

	points[6] = posX + (mSize.x * (1 - mOrigin.x));		points[7] = posY - (mSize.y * mOrigin.y);
	points[8] = posX - (mSize.x * mOrigin.x);		points[9] = posY + (mSize.y * (1 - mOrigin.y));
	points[10] = posX + (mSize.x * (1 -mOrigin.x));		points[11] = posY + (mSize.y * (1 - mOrigin.y));



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

void ImageComponent::drawImageArray(GLfloat* points, GLfloat* texs, GLubyte* colors, unsigned int numArrays)
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

void ImageComponent::init()
{
	if(!mPath.empty())
		loadImage(mPath);

	GuiComponent::init();
}

void ImageComponent::deinit()
{
	unloadImage();

	GuiComponent::deinit();
}

bool ImageComponent::hasImage()
{
	return !mPath.empty();
}

unsigned char ImageComponent::getOpacity() { return mOpacity; }
void ImageComponent::setOpacity(unsigned char opacity) { mOpacity = opacity; }

void ImageComponent::copyScreen()
{
	unloadImage();
	
	glReadBuffer(GL_FRONT);

	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	int width = Renderer::getScreenWidth();
	int height = Renderer::getScreenHeight();
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, width, height, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	mTextureSize.x = height;
	mTextureSize.y = height;

	resize();
}
