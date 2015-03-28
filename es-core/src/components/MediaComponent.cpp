#include "MediaComponent.h"

#include <math.h>
#include "Log.h"
#include "Renderer.h"
#include "ThemeData.h"
#include "Util.h"

Eigen::Vector2f MediaComponent::getCenter() const
{
	return Eigen::Vector2f(mPosition.x(), mPosition.y()) - (getSize().cwiseProduct(mOrigin)) + (getSize() / 2);
}

MediaComponent::MediaComponent(Window* window) 
	: GuiComponent(window), mResizeType(RESIZE_TARGET), mFlip(0, 0), mOrigin(0, 0), mTargetSize(0, 0), mColorShift(0xFFFFFFFF)
{
	updateColors();
}

MediaComponent::~MediaComponent()
{
}

void MediaComponent::resize()
{
	if(!getTexture())
		return;

	const Eigen::Vector2f textureSize = getSourceTextureSize();

	if(textureSize.isZero())
		return;

	// SVG rasterization is determined by height (see SVGResource.cpp), and rasterization is done in terms of pixels
	// if rounding is off enough in the rasterization step (for images with extreme aspect ratios), it can cause cutoff when the aspect ratio breaks
	// so, we always make sure the resultant height is an integer to make sure cutoff doesn't happen, and scale width from that 
	// (you'll see this scattered throughout the function)
	// this is probably not the best way, so if you're familiar with this problem and have a better solution, please make a pull request!

	if(mResizeType == RESIZE_MAX)
	{
		mSize = textureSize;

		Eigen::Vector2f resizeScale((mTargetSize.x() / mSize.x()), (mTargetSize.y() / mSize.y()));

		if(resizeScale.x() < resizeScale.y())
		{
			mSize[0] *= resizeScale.x();
			mSize[1] *= resizeScale.x();
		}
		else{
			mSize[0] *= resizeScale.y();
			mSize[1] *= resizeScale.y();
		}

		// for SVG rasterization, always calculate width from rounded height (see comment above)
		mSize[1] = round(mSize[1]);
		mSize[0] = (mSize[1] / textureSize.y()) * textureSize.x();

	}else{ // mResizeType == RESIZE_TARGET
		// if both components are set, we just stretch
		// if no components are set, we don't resize at all
		mSize = mTargetSize.isZero() ? textureSize : mTargetSize;

		// if only one component is set, we resize in a way that maintains aspect ratio
		// for SVG rasterization, we always calculate width from rounded height (see comment above)
		if(!mTargetSize.x() && mTargetSize.y())
		{
			mSize[1] = round(mTargetSize.y());
			mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
		}
		else if(mTargetSize.x() && !mTargetSize.y())
		{
			mSize[1] = round((mTargetSize.x() / textureSize.x()) * textureSize.y());
			mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
		}
	}

	onSizeChanged();
}

void MediaComponent::onSizeChanged()
{
	updateVertices();
}

void MediaComponent::setOrigin(Eigen::Vector2f origin)
{
	mOrigin = origin;
	updateVertices();
}

void MediaComponent::setResize(Eigen::Vector2f target)
{
	mTargetSize = target;
	mResizeType = RESIZE_TARGET;
	resize();
}

void MediaComponent::setMaxSize(Eigen::Vector2f size)
{
	mTargetSize = size;
	mResizeType = RESIZE_MAX;
	resize();
}

void MediaComponent::setFlip(bool flipX, bool flipY)
{
	mFlip << flipX, flipY;
	updateVertices();
}

void MediaComponent::setColorShift(unsigned int color)
{
	mColorShift = color;
	updateColors();
}

void MediaComponent::setOpacity(unsigned char opacity)
{
	mOpacity = opacity;
	mColorShift = (mColorShift >> 8 << 8) | mOpacity;
	updateColors();
}

bool MediaComponent::hasImage()
{
	return getTexture() != 0;
}

void MediaComponent::onTextureChanged()
{
	resize();
}

Eigen::Vector2f MediaComponent::getSourceTextureSize()
{
	return getTextureSize().cast<float>();
}

void MediaComponent::updateVertices()
{
	if(!getTexture())
		return;

	// we go through this mess to make sure everything is properly rounded
	// if we just round vertices at the end, edge cases occur near sizes of 0.5
	Eigen::Vector2f topLeft(-mSize.x() * mOrigin.x(), -mSize.y() * mOrigin.y());
	Eigen::Vector2f bottomRight(mSize.x() * (1 - mOrigin.x()), mSize.y() * (1 - mOrigin.y()));

	const float width = round(bottomRight.x() - topLeft.x());
	const float height = round(bottomRight.y() - topLeft.y());

	topLeft[0] = floor(topLeft[0]);
	topLeft[1] = floor(topLeft[1]);
	bottomRight[0] = topLeft[0] + width;
	bottomRight[1] = topLeft[1] + height;

	mVertices[0].pos << topLeft.x(), topLeft.y();
	mVertices[1].pos << topLeft.x(), bottomRight.y();
	mVertices[2].pos << bottomRight.x(), topLeft.y();

	mVertices[3].pos << bottomRight.x(), topLeft.y();
	mVertices[4].pos << topLeft.x(), bottomRight.y();
	mVertices[5].pos << bottomRight.x(), bottomRight.y();

	float px, py;
	if(isTextureTiled())
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

	if(mFlip.x())
	{
		for(int i = 0; i < 6; i++)
			mVertices[i].tex[0] = mVertices[i].tex[0] == px ? 0 : px;
	}
	if(mFlip.y())
	{
		for(int i = 1; i < 6; i++)
			mVertices[i].tex[1] = mVertices[i].tex[1] == py ? 0 : py;
	}
}

void MediaComponent::updateColors()
{
	Renderer::buildGLColorArray(mColors, mColorShift, 6);
}

void MediaComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = roundMatrix(parentTrans * getTransform());
	Renderer::setMatrix(trans);

	const GLuint texture = getTexture();
	if(texture && mOpacity > 0)
	{
		// actually draw the image
		glBindTexture(GL_TEXTURE_2D, texture);

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
	}

	GuiComponent::renderChildren(trans);
}

void MediaComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "");
	if(!elem)
		return;

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

	if(properties & COLOR && elem->has("color"))
		setColorShift(elem->get<unsigned int>("color"));
}

std::vector<HelpPrompt> MediaComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> ret;
	ret.push_back(HelpPrompt("a", "select"));
	return ret;
}
