#include "components/ImageComponent.h"

#include "resources/TextureResource.h"
#include "Log.h"
#include "Renderer.h"
#include "Settings.h"
#include "ThemeData.h"

Vector2i ImageComponent::getTextureSize() const
{
	if(mTexture)
		return mTexture->getSize();
	else
		return Vector2i::Zero();
}

Vector2f ImageComponent::getSize() const
{
	return GuiComponent::getSize() * (mBottomRightCrop - mTopLeftCrop);
}

ImageComponent::ImageComponent(Window* window, bool forceLoad, bool dynamic) : GuiComponent(window),
	mTargetIsMax(false), mTargetIsMin(false), mFlipX(false), mFlipY(false), mTargetSize(0, 0), mColorShift(0xFFFFFFFF),
	mForceLoad(forceLoad), mDynamic(dynamic), mFadeOpacity(0), mFading(false), mRotateByTargetSize(false),
	mTopLeftCrop(0.0f, 0.0f), mBottomRightCrop(1.0f, 1.0f)
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

	const Vector2f textureSize = mTexture->getSourceImageSize();
	if(textureSize == Vector2f::Zero())
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

			Vector2f resizeScale((mTargetSize.x() / mSize.x()), (mTargetSize.y() / mSize.y()));

			if(resizeScale.x() < resizeScale.y())
			{
				mSize[0] *= resizeScale.x(); // this will be mTargetSize.x(). We can't exceed it, nor be lower than it.
				// we need to make sure we're not creating an image larger than max size
				mSize[1] = Math::min(Math::round(mSize[1] *= resizeScale.x()), mTargetSize.y());
			}else{
				mSize[1] = Math::round(mSize[1] * resizeScale.y()); // this will be mTargetSize.y(). We can't exceed it.
				
				// for SVG rasterization, always calculate width from rounded height (see comment above)
				// we need to make sure we're not creating an image larger than max size
				mSize[0] = Math::min((mSize[1] / textureSize.y()) * textureSize.x(), mTargetSize.x());
			}
		}else if(mTargetIsMin)
		{
			mSize = textureSize;

			Vector2f resizeScale((mTargetSize.x() / mSize.x()), (mTargetSize.y() / mSize.y()));

			if(resizeScale.x() > resizeScale.y())
			{
				mSize[0] *= resizeScale.x();
				mSize[1] *= resizeScale.x();

				float cropPercent = (mSize.y() - mTargetSize.y()) / (mSize.y() * 2);
				crop(0, cropPercent, 0, cropPercent);
			}else{
				mSize[0] *= resizeScale.y();
				mSize[1] *= resizeScale.y();

				float cropPercent = (mSize.x() - mTargetSize.x()) / (mSize.x() * 2);
				crop(cropPercent, 0, cropPercent, 0);
			}

			// for SVG rasterization, always calculate width from rounded height (see comment above)
			// we need to make sure we're not creating an image smaller than min size
			mSize[1] = Math::max(Math::round(mSize[1]), mTargetSize.y());
			mSize[0] = Math::max((mSize[1] / textureSize.y()) * textureSize.x(), mTargetSize.x());

		}else{
			// if both components are set, we just stretch
			// if no components are set, we don't resize at all
			mSize = mTargetSize == Vector2f::Zero() ? textureSize : mTargetSize;

			// if only one component is set, we resize in a way that maintains aspect ratio
			// for SVG rasterization, we always calculate width from rounded height (see comment above)
			if(!mTargetSize.x() && mTargetSize.y())
			{
				mSize[1] = Math::round(mTargetSize.y());
				mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
			}else if(mTargetSize.x() && !mTargetSize.y())
			{
				mSize[1] = Math::round((mTargetSize.x() / textureSize.x()) * textureSize.y());
				mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
			}
		}
	}

	mSize[0] = Math::round(mSize.x());
	mSize[1] = Math::round(mSize.y());
	// mSize.y() should already be rounded
	mTexture->rasterizeAt((size_t)mSize.x(), (size_t)mSize.y());

	onSizeChanged();
}

void ImageComponent::onSizeChanged()
{
	updateVertices();
}

void ImageComponent::setDefaultImage(std::string path)
{
	mDefaultPath = path;
}

void ImageComponent::setImage(std::string path, bool tile)
{
	if(path.empty() || !ResourceManager::getInstance()->fileExists(path))
	{
		if(mDefaultPath.empty() || !ResourceManager::getInstance()->fileExists(mDefaultPath))
			mTexture.reset();
		else
			mTexture = TextureResource::get(mDefaultPath, tile, mForceLoad, mDynamic);
	} else {
		mTexture = TextureResource::get(path, tile, mForceLoad, mDynamic);
	}

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
	mTargetSize = Vector2f(width, height);
	mTargetIsMax = false;
	mTargetIsMin = false;
	resize();
}

void ImageComponent::setMaxSize(float width, float height)
{
	mTargetSize = Vector2f(width, height);
	mTargetIsMax = true;
	mTargetIsMin = false;
	resize();
}

void ImageComponent::setMinSize(float width, float height)
{
	mTargetSize = Vector2f(width, height);
	mTargetIsMax = false;
	mTargetIsMin = true;
	resize();
}

Vector2f ImageComponent::getRotationSize() const
{
	return mRotateByTargetSize ? mTargetSize : mSize;
}

void ImageComponent::setRotateByTargetSize(bool rotate)
{
	mRotateByTargetSize = rotate;
}

void ImageComponent::cropLeft(float percent)
{
	assert(percent >= 0.0f && percent <= 1.0f);
	mTopLeftCrop.x() = percent;
}

void ImageComponent::cropTop(float percent)
{
	assert(percent >= 0.0f && percent <= 1.0f);
	mTopLeftCrop.y() = percent;
}

void ImageComponent::cropRight(float percent)
{
	assert(percent >= 0.0f && percent <= 1.0f);
	mBottomRightCrop.x() = 1.0f - percent;
}

void ImageComponent::cropBot(float percent)
{
	assert(percent >= 0.0f && percent <= 1.0f);
	mBottomRightCrop.y() = 1.0f - percent;
}

void ImageComponent::crop(float left, float top, float right, float bot)
{
	cropLeft(left);
	cropTop(top);
	cropRight(right);
	cropBot(bot);
}

void ImageComponent::uncrop()
{
	crop(0, 0, 0, 0);
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
	Vector2f size(Math::round(mSize.x()), Math::round(mSize.y()));
	Vector2f topLeft(size * mTopLeftCrop);
	Vector2f bottomRight(size * mBottomRightCrop);

	mVertices[0].pos = Vector2f(topLeft.x(), topLeft.y());
	mVertices[1].pos = Vector2f(topLeft.x(), bottomRight.y());
	mVertices[2].pos = Vector2f(bottomRight.x(), topLeft.y());

	mVertices[3].pos = Vector2f(bottomRight.x(), topLeft.y());
	mVertices[4].pos = Vector2f(topLeft.x(), bottomRight.y());
	mVertices[5].pos = Vector2f(bottomRight.x(), bottomRight.y());

	float px, py;
	if(mTexture->isTiled())
	{
		px = mSize.x() / getTextureSize().x();
		py = mSize.y() / getTextureSize().y();
	}else{
		px = 1;
		py = 1;
	}

	mVertices[0].tex = Vector2f(mTopLeftCrop.x(), py - mTopLeftCrop.y());
	mVertices[1].tex = Vector2f(mTopLeftCrop.x(), 1 - mBottomRightCrop.y());
	mVertices[2].tex = Vector2f(px * mBottomRightCrop.x(), py - mTopLeftCrop.y());

	mVertices[3].tex = Vector2f(px * mBottomRightCrop.x(), py - mTopLeftCrop.y());
	mVertices[4].tex = Vector2f(mTopLeftCrop.x(), 1 - mBottomRightCrop.y());
	mVertices[5].tex = Vector2f(px * mBottomRightCrop.x(), 1 - mBottomRightCrop.y());

	if(mFlipX)
	{
		for(int i = 0; i < 6; i++)
			mVertices[i].tex[0] = px - mVertices[i].tex[0];
	}
	if(mFlipY)
	{
		for(int i = 0; i < 6; i++)
			mVertices[i].tex[1] = py - mVertices[i].tex[1];
	}
}

void ImageComponent::updateColors()
{
	Renderer::buildGLColorArray(mColors, mColorShift, 6);
}

void ImageComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	if(mTexture && mOpacity > 0)
	{
		if(Settings::getInstance()->getBool("DebugImage")) {
			Vector2f targetSizePos = (mTargetSize - mSize) * mOrigin * -1;
			Renderer::drawRect(targetSizePos.x(), targetSizePos.y(), mTargetSize.x(), mTargetSize.y(), 0xFF000033);
			Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0x00000033);
		}
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

	Vector2f scale = getParent() ? getParent()->getSize() : Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	
	if(properties & POSITION && elem->has("pos"))
	{
		Vector2f denormalized = elem->get<Vector2f>("pos") * scale;
		setPosition(Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & ThemeFlags::SIZE)
	{
		if(elem->has("size"))
			setResize(elem->get<Vector2f>("size") * scale);
		else if(elem->has("maxSize"))
			setMaxSize(elem->get<Vector2f>("maxSize") * scale);
		else if(elem->has("minSize"))
			setMinSize(elem->get<Vector2f>("minSize") * scale);
	}

	// position + size also implies origin
	if((properties & ORIGIN || (properties & POSITION && properties & ThemeFlags::SIZE)) && elem->has("origin"))
		setOrigin(elem->get<Vector2f>("origin"));

	if(elem->has("default")) {
		setDefaultImage(elem->get<std::string>("default"));
	}

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
			setRotationOrigin(elem->get<Vector2f>("rotationOrigin"));
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
