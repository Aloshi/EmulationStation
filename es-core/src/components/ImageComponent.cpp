#include "components/ImageComponent.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>
#include "Log.h"
#include "Renderer.h"
#include "ThemeData.h"
#include "Util.h"

Eigen::Vector2i ImageComponent::getTextureSize() const
{
	if(mTexture)
		return mTexture->getSize();
	else
		return Eigen::Vector2i::Zero();
}

ImageComponent::ImageComponent(Window* window, bool forceLoad, bool dynamic) : GuiComponent(window),
	mTargetIsMax(false), mFlipX(false), mFlipY(false), mTargetSize(0, 0), mColorShift(0xFFFFFFFF),
	mForceLoad(forceLoad), mDynamic(dynamic), mFadeOpacity(0.0f), mFading(false)
{
	updateColors();
}

ImageComponent::~ImageComponent()
{
}

void ImageComponent::resize()
{
	if(!mTexture)
		return;

	const Eigen::Vector2f textureSize = mTexture->getSourceImageSize();
	if(textureSize.isZero())
		return;

	if(mTexture->isTiled())
	{
		mSize = mTargetSize;
	}else{
		// SVG rasterization is determined by height (see SVGResource.cpp), and rasterization is done in terms of pixels
		// if rounding is off enough in the rasterization step (for images with extreme aspect ratios), it can cause cutoff when the aspect ratio breaks
		// so, we always make sure the resultant height is an integer to make sure cutoff doesn't happen, and scale width from that 
		// (you'll see this scattered throughout the function)
		// this is probably not the best way, so if you're familiar with this problem and have a better solution, please make a pull request!

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

			// for SVG rasterization, always calculate width from rounded height (see comment above)
			mSize[1] = round(mSize[1]);
			mSize[0] = (mSize[1] / textureSize.y()) * textureSize.x();

		}else{
			// if both components are set, we just stretch
			// if no components are set, we don't resize at all
			mSize = mTargetSize.isZero() ? textureSize : mTargetSize;

			// if only one component is set, we resize in a way that maintains aspect ratio
			// for SVG rasterization, we always calculate width from rounded height (see comment above)
			if(!mTargetSize.x() && mTargetSize.y())
			{
				mSize[1] = round(mTargetSize.y());
				mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
			}else if(mTargetSize.x() && !mTargetSize.y())
			{
				mSize[1] = round((mTargetSize.x() / textureSize.x()) * textureSize.y());
				mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
			}
		}
	}
	// mSize.y() should already be rounded
	mTexture->rasterizeAt((int)round(mSize.x()), (int)round(mSize.y()));

	onSizeChanged();
}

void ImageComponent::onSizeChanged()
{
	updateVertices();
}

void ImageComponent::setImage(std::string path, bool tile)
{
	if(path.empty() || !ResourceManager::getInstance()->fileExists(path))
		mTexture.reset();
	else
		mTexture = TextureResource::get(path, tile, mForceLoad, mDynamic);

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
	updateVertices();
}

void ImageComponent::setFlipY(bool flip)
{
	mFlipY = flip;
	updateVertices();
}

void ImageComponent::setColorShift(unsigned int color)
{
	mColorShift = color;
	// Grab the opacity from the color shift because we may need to apply it if
	// fading textures in
	mOpacity = color & 0xff;
	updateColors();
}

void ImageComponent::setOpacity(unsigned char opacity)
{
	mOpacity = opacity;
	mColorShift = (mColorShift >> 8 << 8) | mOpacity;
	updateColors();
}

void ImageComponent::updateVertices()
{
	if(!mTexture || !mTexture->isInitialized())
		return;

	// we go through this mess to make sure everything is properly rounded
	// if we just round vertices at the end, edge cases occur near sizes of 0.5
	Eigen::Vector2f topLeft(0.0, 0.0);
	Eigen::Vector2f bottomRight(round(mSize.x()), round(mSize.y()));

	mVertices[0].pos << topLeft.x(), topLeft.y();
	mVertices[1].pos << topLeft.x(), bottomRight.y();
	mVertices[2].pos << bottomRight.x(), topLeft.y();

	mVertices[3].pos << bottomRight.x(), topLeft.y();
	mVertices[4].pos << topLeft.x(), bottomRight.y();
	mVertices[5].pos << bottomRight.x(), bottomRight.y();

	float px, py;
	if(mTexture->isTiled())
	{
		px = mSize.x() / getTextureSize().x();
		py = mSize.y() / getTextureSize().y();
	}else{
		px = 1;
		py = 1;
	}

	mVertices[0].tex << 0, py;
	mVertices[1].tex << 0, 0;
	mVertices[2].tex << px, py;

	mVertices[3].tex << px, py;
	mVertices[4].tex << 0, 0;
	mVertices[5].tex << px, 0;

	if(mFlipX)
	{
		for(int i = 0; i < 6; i++)
			mVertices[i].tex[0] = mVertices[i].tex[0] == px ? 0 : px;
	}
	if(mFlipY)
	{
		for(int i = 1; i < 6; i++)
			mVertices[i].tex[1] = mVertices[i].tex[1] == py ? 0 : py;
	}
}

void ImageComponent::updateColors()
{
	Renderer::buildGLColorArray(mColors, mColorShift, 6);
}

void ImageComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	if(mTexture && mOpacity > 0)
	{
		if(mTexture->isInitialized())
		{
			// actually draw the image
			// The bind() function returns false if the texture is not currently loaded. A blank
			// texture is bound in this case but we want to handle a fade so it doesn't just 'jump' in
			// when it finally loads
			fadeIn(mTexture->bind());

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].pos);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].tex);
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, mColors);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
		}else{
			LOG(LogError) << "Image texture is not initialized!";
			mTexture.reset();
		}
	}

	GuiComponent::renderChildren(trans);
}

void ImageComponent::fadeIn(bool textureLoaded)
{
	if (!mForceLoad)
	{
		if (!textureLoaded)
		{
			// Start the fade if this is the first time we've encountered the unloaded texture
			if (!mFading)
			{
				// Start with a zero opacity and flag it as fading
				mFadeOpacity = 0;
				mFading = true;
				// Set the colours to be translucent
				mColorShift = (mColorShift >> 8 << 8) | 0;
				updateColors();
			}
		}
		else if (mFading)
		{
			// The texture is loaded and we need to fade it in. The fade is based on the frame rate
			// and is 1/4 second if running at 60 frames per second although the actual value is not
			// that important
			int opacity = mFadeOpacity + 255 / 15;
			// See if we've finished fading
			if (opacity >= 255)
			{
				mFadeOpacity = 255;
				mFading = false;
			}
			else
			{
				mFadeOpacity = (unsigned char)opacity;
			}
			// Apply the combination of the target opacity and current fade
			float newOpacity = (float)mOpacity * ((float)mFadeOpacity / 255.0f);
			mColorShift = (mColorShift >> 8 << 8) | (unsigned char)newOpacity;
			updateColors();
		}
	}
}

bool ImageComponent::hasImage()
{
	return (bool)mTexture;
}

void ImageComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "image");
	if(!elem)
	{
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

	if(properties & COLOR && elem->has("color"))
		setColorShift(elem->get<unsigned int>("color"));

	if(properties & ThemeFlags::ROTATION) {
		if(elem->has("rotation"))
			setRotationDegrees(elem->get<float>("rotation"));
		if(elem->has("rotationOrigin"))
			setRotationOrigin(elem->get<Eigen::Vector2f>("rotationOrigin"));
	}

	if(properties & ThemeFlags::Z_INDEX && elem->has("zIndex"))
		setZIndex(elem->get<float>("zIndex"));
	else
		setZIndex(getDefaultZIndex());
}

std::vector<HelpPrompt> ImageComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> ret;
	ret.push_back(HelpPrompt("a", "select"));
	return ret;
}
