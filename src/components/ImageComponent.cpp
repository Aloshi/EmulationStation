#include "ImageComponent.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>
#include "../Log.h"
#include "../Renderer.h"
#include "../Window.h"

Eigen::Vector2i ImageComponent::getTextureSize() const
{
	if(mTexture)
		return mTexture->getSize();
	else
		return Eigen::Vector2i(0, 0);
}

Eigen::Vector2f ImageComponent::getCenter() const
{
	return Eigen::Vector2f(mPosition.x() - (getSize().x() * mOrigin.x()) + getSize().x() / 2, 
		mPosition.y() - (getSize().y() * mOrigin.y()) + getSize().y() / 2);
}

ImageComponent::ImageComponent(Window* window, float offsetX, float offsetY, std::string path, float targetWidth, float targetHeight, bool allowUpscale) : GuiComponent(window), 
	mTiled(false), mAllowUpscale(allowUpscale), mFlipX(false), mFlipY(false), mOrigin(0.5, 0.5), mTargetSize(targetWidth, targetHeight)
{
	setPosition(offsetX, offsetY);

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

	mSize << (float)getTextureSize().x(), (float)getTextureSize().y();
	
	//(we don't resize tiled images)
	if(!mTiled && (mTargetSize.x() || mTargetSize.y()))
	{
		Eigen::Vector2f resizeScale(Eigen::Vector2f::Zero());

		if(mTargetSize.x() && (mAllowUpscale || mSize.x() > mTargetSize.x()))
		{
			resizeScale[0] = mTargetSize.x() / mSize.x();
		}
		if(mTargetSize.y() && (mAllowUpscale || mSize.y() > mTargetSize.y()))
		{
			resizeScale[1] = mTargetSize.y() / mSize.y();
		}

		if(resizeScale.x() && !resizeScale.y())
			resizeScale[1] = resizeScale.x();
		if(resizeScale[1] && !resizeScale.x())
			resizeScale[0] = resizeScale.y();

		if(resizeScale.x())
			mSize[0] = (mSize.x() * resizeScale.x());
		if(resizeScale.y())
			mSize[1] = (mSize.y() * resizeScale.y());
	}

	if(mTiled)
		mSize = mTargetSize;
}

void ImageComponent::setImage(std::string path)
{
	mPath = path;

	if(mPath.empty() || !mWindow->getResourceManager()->fileExists(mPath))
		mTexture.reset();
	else
		mTexture = TextureResource::get(*mWindow->getResourceManager(), mPath);

	resize();
}

void ImageComponent::setOrigin(float originX, float originY)
{
	mOrigin << originX, originY;
}

void ImageComponent::setTiling(bool tile)
{
	mTiled = tile;

	if(mTiled)
		mAllowUpscale = false;

	resize();
}

void ImageComponent::setResize(float width, float height, bool allowUpscale)
{
	mTargetSize << width, height;
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

void ImageComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);
	
	if(mTexture && getOpacity() > 0)
	{
		GLfloat points[12], texs[12];
		GLubyte colors[6*4];

		if(mTiled)
		{
			float xCount = mSize.x() / getTextureSize().x();
			float yCount = mSize.y() / getTextureSize().y();
			
			Renderer::buildGLColorArray(colors, 0xFFFFFF00 | (getOpacity()), 6);
			buildImageArray(0, 0, points, texs, xCount, yCount);
		}else{
			Renderer::buildGLColorArray(colors, 0xFFFFFF00 | (getOpacity()), 6);
			buildImageArray(0, 0, points, texs);
		}

		drawImageArray(points, texs, colors, 6);
	}

	GuiComponent::renderChildren(trans);
}

void ImageComponent::buildImageArray(int posX, int posY, GLfloat* points, GLfloat* texs, float px, float py)
{
	points[0] = posX - (mSize.x() * mOrigin.x());		points[1] = posY - (mSize.y() * mOrigin.y());
	points[2] = posX - (mSize.x() * mOrigin.x());		points[3] = posY + (mSize.y() * (1 - mOrigin.y()));
	points[4] = posX + (mSize.x() * (1 - mOrigin.x()));		points[5] = posY - (mSize.y() * mOrigin.y());

	points[6] = posX + (mSize.x() * (1 - mOrigin.x()));		points[7] = posY - (mSize.y() * mOrigin.y());
	points[8] = posX - (mSize.x() * mOrigin.x());		points[9] = posY + (mSize.y() * (1 - mOrigin.y()));
	points[10] = posX + (mSize.x() * (1 -mOrigin.x()));		points[11] = posY + (mSize.y() * (1 - mOrigin.y()));



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
