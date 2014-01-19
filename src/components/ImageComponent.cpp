#include "ImageComponent.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>
#include "../Log.h"
#include "../Renderer.h"
#include "../ThemeData.h"

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

ImageComponent::ImageComponent(Window* window, const Eigen::Vector2f& pos, const std::string& path) : GuiComponent(window), 
	mTargetIsMax(false), mFlipX(false), mFlipY(false), mOrigin(0.0, 0.0), mTargetSize(0, 0), mColorShift(0xFFFFFFFF)
{
	setPosition(pos.x(), pos.y());

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

	const Eigen::Vector2f textureSize((float)getTextureSize().x(), (float)getTextureSize().y());

	if(mTexture->isTiled())
	{
		mSize = mTargetSize;
	}else{
		if(mTargetIsMax)
		{
			mSize = textureSize;

			Eigen::Vector2f resizeScale((mTargetSize.x() / mSize.x()), (mTargetSize.y() / mSize.y()));
			
			if(resizeScale.x() < resizeScale.y())
			{
				mSize[0] *= resizeScale.x();
				mSize[1] *= resizeScale.x();
			}else{
				mSize[0] *= resizeScale.y();
				mSize[1] *= resizeScale.y();
			}

		}else{
			// if both components are set, we just stretch
			// if no components are set, we don't resize at all
			mSize = mTargetSize.isZero() ? textureSize : mTargetSize;

			// if only one component is set, we resize in a way that maintains aspect ratio
			if(!mTargetSize.x() && mTargetSize.y())
			{
				mSize[0] = (mTargetSize.y() / textureSize.y()) * textureSize.x();
				mSize[1] = mTargetSize.y();
			}else if(mTargetSize.x() && !mTargetSize.y())
			{
				mSize[0] = mTargetSize.x();
				mSize[1] = (mTargetSize.x() / textureSize.x()) * textureSize.y();
			}
		}
	}
}

void ImageComponent::setImage(std::string path, bool tile)
{
	if(path.empty() || !ResourceManager::getInstance()->fileExists(path))
		mTexture.reset();
	else
		mTexture = TextureResource::get(path, tile);

	resize();
}

void ImageComponent::setImage(const char* path, size_t length, bool tile)
{
	mTexture.reset();

	mTexture = TextureResource::get("", tile);
	mTexture->initFromMemory(path, length);
	
	resize();
}

void ImageComponent::setImage(const std::shared_ptr<TextureResource>& texture)
{
	mTexture = texture;
	resize();
}

void ImageComponent::setOrigin(float originX, float originY)
{
	mOrigin << originX, originY;
}

void ImageComponent::setResize(float width, float height)
{
	mTargetSize << width, height;
	mTargetIsMax = false;
	resize();
}

void ImageComponent::setMaxSize(float width, float height)
{
	mTargetSize << width, height;
	mTargetIsMax = true;
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

void ImageComponent::setColorShift(unsigned int color)
{
	mColorShift = color;
}

void ImageComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);
	
	if(mTexture && getOpacity() > 0)
	{
		GLfloat points[12], texs[12];
		GLubyte colors[6*4];

		if(mTexture->isTiled())
		{
			float xCount = mSize.x() / getTextureSize().x();
			float yCount = mSize.y() / getTextureSize().y();
			
			Renderer::buildGLColorArray(colors, (mColorShift >> 8 << 8)| (getOpacity()), 6);
			buildImageArray(0, 0, points, texs, xCount, yCount);
		}else{
			Renderer::buildGLColorArray(colors, (mColorShift >> 8 << 8) | (getOpacity()), 6);
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
	return (bool)mTexture;
}

void ImageComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	LOG(LogInfo) << " req image [" << view << "." << element << "]  (flags: " << properties << ")";

	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "image");
	if(!elem)
	{
		LOG(LogInfo) << "    (missing)";
		return;
	}

	Eigen::Vector2f scale = getParent() ? getParent()->getSize() : Eigen::Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	
	if(properties & POSITION && elem->has("pos"))
	{
		Eigen::Vector2f denormalized = elem->get<Eigen::Vector2f>("pos").cwiseProduct(scale);
		setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & ThemeFlags::SIZE)
	{
		if(elem->has("size"))
			setResize(elem->get<Eigen::Vector2f>("size").cwiseProduct(scale));
		else if(elem->has("maxSize"))
			setMaxSize(elem->get<Eigen::Vector2f>("maxSize").cwiseProduct(scale));
	}

	// position + size also implies origin
	if((properties & ORIGIN || (properties & POSITION && properties & ThemeFlags::SIZE)) && elem->has("origin"))
		setOrigin(elem->get<Eigen::Vector2f>("origin"));

	if(properties & PATH && elem->has("path"))
	{
		bool tile = (elem->has("tile") && elem->get<bool>("tile"));
		setImage(elem->get<std::string>("path"), tile);
	}
}
