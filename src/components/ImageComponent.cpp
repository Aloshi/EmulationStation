#include "ImageComponent.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>
#include "../Log.h"
#include "../Renderer.h"
#include "../Window.h"

Vector2u ImageComponent::getTextureSize() 
{
	if(mTexture)
		return mTexture->getSize();
	else
		return Vector2u(0, 0);
}

ImageComponent::ImageComponent(Window* window, int offsetX, int offsetY, std::string path, unsigned int resizeWidth, unsigned int resizeHeight, bool allowUpscale) : GuiComponent(window)
{
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
}

void ImageComponent::resize()
{
	if(!mTexture)
		return;

	mSize.x = getTextureSize().x;
	mSize.y = getTextureSize().y;

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
		mSize = mTargetSize;
}

void ImageComponent::setImage(std::string path)
{
	if(mPath == path)
		return;

	mPath = path;

	if(mPath.empty() || !mWindow->getResourceManager()->fileExists(mPath))
		mTexture.reset();
	else
		mTexture = TextureResource::get(*mWindow->getResourceManager(), mPath);

	resize();
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
	if(mTexture && getOpacity() > 0)
	{
		GLfloat points[12], texs[12];
		GLubyte colors[6*4];

		if(mTiled)
		{
			float xCount = (float)mSize.x / getTextureSize().x;
			float yCount = (float)mSize.y / getTextureSize().y;
			
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
	mTexture->bind();

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

bool ImageComponent::hasImage()
{
	return !mPath.empty();
}


void ImageComponent::copyScreen()
{
	mTexture.reset();

	mTexture = TextureResource::get(*mWindow->getResourceManager(), "");
	mTexture->initFromScreen();

	resize();
}
