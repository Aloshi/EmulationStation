#include "GuiImage.h"
#include <iostream>
#include <boost/filesystem.hpp>

int GuiImage::getWidth() { return mWidth; }
int GuiImage::getHeight() { return mHeight; }

GuiImage::GuiImage(int offsetX, int offsetY, std::string path, unsigned int maxWidth, unsigned int maxHeight, bool resizeExact)
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

	mMaxWidth = maxWidth;
	mMaxHeight = maxHeight;

	mResizeExact = resizeExact;
	mUseAlpha = false;

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
		std::cout << "Loading image...";
		image = FreeImage_Load(format, path.c_str());
		std::cout << "success\n";
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



	//force power of two for testing
	//width = 512; height = 512;

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

	std::cout << "Image load successful, w: " << mWidth << " h: " << mHeight << " texID: " << mTextureID << "\n";
}

void GuiImage::unloadImage()
{
	if(mTextureID)
	{
		std::cout << "deleting texture\n";
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

void GuiImage::setAlpha(bool useAlpha)
{
	mUseAlpha = useAlpha;
}

void GuiImage::onRender()
{
	if(mTextureID)
	{
		glBindTexture(GL_TEXTURE_2D, mTextureID);
		glEnable(GL_TEXTURE_2D);


		GLfloat points[12];
		points[0] = mOffsetX - (mWidth * mOriginX);		points[1] = mOffsetY - (mHeight * mOriginY);
		points[2] = mOffsetX - (mWidth * mOriginX);		points[3] = mOffsetY + (mHeight * (1 - mOriginY));
		points[4] = mOffsetX + (mWidth * (1 - mOriginX));	points[5] = mOffsetY - (mHeight * mOriginY);

		points[6] = mOffsetX + (mWidth * (1 - mOriginX));	points[7] = mOffsetY - (mHeight * mOriginY);
		points[8] = mOffsetX - (mWidth * mOriginX);		points[9] = mOffsetY + (mHeight * (1 - mOriginY));
		points[10] = mOffsetX + (mWidth * (1 -mOriginX));	points[11] = mOffsetY + (mHeight * (1 - mOriginY));

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
	}
}
