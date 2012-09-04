#include "GuiImage.h"
#include <iostream>
#include <boost/filesystem.hpp>

int GuiImage::getWidth() { return mWidth; }
int GuiImage::getHeight() { return mHeight; }

GuiImage::GuiImage(int offsetX, int offsetY, std::string path, unsigned int resizeWidth, unsigned int resizeHeight, bool resizeExact)
{
	mTextureID = 0;

	mOffsetX = offsetX;
	mOffsetY = offsetY;

	//default origin (center of image)
	mOriginX = 0.5;
	mOriginY = 0.5;

	mWidth = 0;
	mHeight = 0;

	mTiled = false;

	mResizeWidth = resizeWidth;
	mResizeHeight = resizeHeight;

	mResizeExact = resizeExact;

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
	//format = FreeImage_GetFileType(path.c_str(), 0);
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

	imageData = FreeImage_GetBits(image);
	if(!imageData)
	{
		std::cerr << "Error retriving bits from image \"" << path << "\"!\n";
		return;
	}



	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	if(!width || !height)
	{
		std::cerr << "Width or height are zero for image \"" << path << "\"!\n";
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	mWidth = width;
	mHeight = height;

	//free the image data
	FreeImage_Unload(image);

	//free the memory from that pointer
	delete[] imageRGBA;

	//a simple way to resize: lie about our real texture size!
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
			mWidth *= resizeScaleX;
		if(resizeScaleY)
			mHeight *= resizeScaleY;
	}

	//std::cout << "Image load successful, w: " << mWidth << " h: " << mHeight << " texID: " << mTextureID << "\n";
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

void GuiImage::onRender()
{
	if(mTextureID)
	{
		if(mTiled)
		{
			for(unsigned int x = 0; x < (unsigned int)((float)mResizeWidth/mWidth + 1.5); x++)
			{
				for(unsigned int y = 0; y < (unsigned int)((float)mResizeHeight/mHeight + 1.5); y++)
				{
					drawImage(mOffsetX + x*mWidth, mOffsetY + y*mHeight);
				}
			}
		}else{
			drawImage(mOffsetX, mOffsetY);
		}
	}
}

void GuiImage::drawImage(int posX, int posY)
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat points[12];
	points[0] = posX - (mWidth * mOriginX);		points[1] = posY - (mHeight * mOriginY);
	points[2] = posX - (mWidth * mOriginX);		points[3] = posY + (mHeight * (1 - mOriginY));
	points[4] = posX + (mWidth * (1 - mOriginX));	points[5] = posY - (mHeight * mOriginY);

	points[6] = posX + (mWidth * (1 - mOriginX));	points[7] = posY - (mHeight * mOriginY);
	points[8] = posX - (mWidth * mOriginX);		points[9] = posY + (mHeight * (1 - mOriginY));
	points[10] = posX + (mWidth * (1 -mOriginX));	points[11] = posY + (mHeight * (1 - mOriginY));

	//std::cout << "x: " << points[0] << " y: " << points[1] << " to x: " << points[10] << " y: " << points[11] << std::endl;
	//std::cout << "(w: " << mWidth << " h: " << mHeight << ")" << std::endl;

	GLfloat texs[12];
	texs[0] = 0;	texs[1] = 1;
	texs[2] = 0;	texs[3] = 0;
	texs[4] = 1;	texs[5] = 1;

	texs[6] = 1;	texs[7] = 1;
	texs[8] = 0;	texs[9] = 0;
	texs[10] = 1;	texs[11] = 0;


	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, points);
	glTexCoordPointer(2, GL_FLOAT, 0, texs);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void GuiImage::onInit()
{
	if(!mPath.empty())
	{
		loadImage(mPath);
	}
}

void GuiImage::onDeinit()
{
	unloadImage();
}
