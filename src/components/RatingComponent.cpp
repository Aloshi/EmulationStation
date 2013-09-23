#include "RatingComponent.h"
#include "../Renderer.h"
#include "../Window.h"

RatingComponent::RatingComponent(Window* window) : GuiComponent(window)
{
	mFilledTexture = TextureResource::get(*window->getResourceManager(), ":/button.png");
	mUnfilledTexture = TextureResource::get(*window->getResourceManager(), ":/button.png");
	mValue = 0.5f;
	mSize << 64 * 5.0f, 64;
	updateVertices();
}

void RatingComponent::setValue(const std::string& value)
{
	mValue = stof(value);
	updateVertices();
}

std::string RatingComponent::getValue() const
{
	return std::to_string(mValue);
}

void RatingComponent::onSizeChanged()
{
	updateVertices();
}

void RatingComponent::updateVertices()
{
	const float numStars = 5.0f;

	float h = getSize().y();
	float w = h * mValue * numStars;
	float fw = h * numStars;

	mVertices[0].pos << 0, 0;
		mVertices[0].tex << 0, 0;
	mVertices[1].pos << w, h;
		mVertices[1].tex << mValue * numStars, 1.0f;
	mVertices[2].pos << 0, h;
		mVertices[2].tex << 0, 1.0f;

	mVertices[3] = mVertices[0];
	mVertices[4].pos << w, 0;
		mVertices[5].tex << mValue * numStars, 0;
	mVertices[5] = mVertices[1];

	mVertices[6] = mVertices[4];
	mVertices[7].pos << fw, h;
		mVertices[7].tex << numStars, 1.0f;
	mVertices[8] = mVertices[1];

	mVertices[9] = mVertices[6];
	mVertices[10].pos << fw, 0;
		mVertices[10].tex << numStars, 0;
	mVertices[11] = mVertices[7];
}

void RatingComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].pos);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].tex);
	//glColorPointer(4, GL_UNSIGNED_BYTE, 0, mColors);

	mFilledTexture->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	mUnfilledTexture->bind();
	glDrawArrays(GL_TRIANGLES, 6, 6);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	renderChildren(trans);
}

