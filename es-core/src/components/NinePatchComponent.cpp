#include "components/NinePatchComponent.h"

#include "resources/TextureResource.h"
#include "Log.h"
#include "Renderer.h"
#include "ThemeData.h"

NinePatchComponent::NinePatchComponent(Window* window, const std::string& path, unsigned int edgeColor, unsigned int centerColor) : GuiComponent(window),
	mCornerSize(16, 16),
	mEdgeColor(edgeColor), mCenterColor(centerColor),
	mPath(path),
	mVertices(NULL), mColors(NULL)
{
	if(!mPath.empty())
		buildVertices();
}

NinePatchComponent::~NinePatchComponent()
{
	if (mVertices != NULL)
		delete[] mVertices;

	if (mColors != NULL)
		delete[] mColors;
}

void NinePatchComponent::updateColors()
{
	Renderer::buildGLColorArray(mColors, mEdgeColor, 6 * 9);
	Renderer::buildGLColorArray(&mColors[4 * 6 * 4], mCenterColor, 6);
}

void NinePatchComponent::buildVertices()
{
	if(mVertices != NULL)
		delete[] mVertices;

	if(mColors != NULL)
		delete[] mColors;

	mTexture = TextureResource::get(mPath);

	if(mTexture->getSize() == Vector2i::Zero())
	{
		mVertices = NULL;
		mColors = NULL;
		LOG(LogWarning) << "NinePatchComponent missing texture!";
		return;
	}

	mVertices = new Vertex[6 * 9];
	mColors = new GLubyte[6 * 9 * 4];
	updateColors();

	const Vector2f texSize = Vector2f((float)mTexture->getSize().x(), (float)mTexture->getSize().y());

	float imgSizeX[3] = {mCornerSize.x(), mSize.x() - mCornerSize.x() * 2, mCornerSize.x()};
	float imgSizeY[3] = {mCornerSize.y(), mSize.y() - mCornerSize.y() * 2, mCornerSize.y()};
	float imgPosX[3] = {0, imgSizeX[0], imgSizeX[0] + imgSizeX[1]};
	float imgPosY[3] = {0, imgSizeY[0], imgSizeY[0] + imgSizeY[1]};

	//the "1 +" in posY and "-" in sizeY is to deal with texture coordinates having a bottom left corner origin vs. verticies having a top left origin
	float texSizeX[3] = {mCornerSize.x() / texSize.x(), (texSize.x() - mCornerSize.x() * 2) / texSize.x(), mCornerSize.x() / texSize.x()};
	float texSizeY[3] = {-mCornerSize.y() / texSize.y(), -(texSize.y() - mCornerSize.y() * 2) / texSize.y(), -mCornerSize.y() / texSize.y()};
	float texPosX[3] = {0, texSizeX[0], texSizeX[0] + texSizeX[1]};
	float texPosY[3] = {1, 1 + texSizeY[0], 1 + texSizeY[0] + texSizeY[1]};

	int v = 0;
	for(int slice = 0; slice < 9; slice++)
	{
		int sliceX = slice % 3;
		int sliceY = slice / 3;

		Vector2f imgPos = Vector2f(imgPosX[sliceX], imgPosY[sliceY]);
		Vector2f imgSize = Vector2f(imgSizeX[sliceX], imgSizeY[sliceY]);

		mVertices[v + 0].pos = imgPos;
		mVertices[v + 1].pos = imgPos + Vector2f(0, imgSize.y());
		mVertices[v + 2].pos = imgPos + Vector2f(imgSize.x(), 0);
		mVertices[v + 3].pos = mVertices[v + 2].pos;
		mVertices[v + 4].pos = mVertices[v + 1].pos;
		mVertices[v + 5].pos = imgPos + imgSize;

		Vector2f texPos = Vector2f(texPosX[sliceX], texPosY[sliceY]);
		Vector2f texSize = Vector2f(texSizeX[sliceX], texSizeY[sliceY]);

		mVertices[v + 0].tex = texPos;
		mVertices[v + 1].tex = texPos + Vector2f(0, texSize.y());
		mVertices[v + 2].tex = texPos + Vector2f(texSize.x(), 0);
		mVertices[v + 3].tex = mVertices[v + 2].tex;
		mVertices[v + 4].tex = mVertices[v + 1].tex;
		mVertices[v + 5].tex = texPos + texSize;

		v += 6;
	}

	// round vertices
	for(int i = 0; i < 6*9; i++)
	{
		mVertices[i].pos.round();
	}
}

void NinePatchComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	trans.round();
	
	if(mTexture && mVertices != NULL)
	{
		Renderer::setMatrix(trans);

		mTexture->bind();

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].tex);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, mColors);

		glDrawArrays(GL_TRIANGLES, 0, 6 * 9);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}

	renderChildren(trans);
}

void NinePatchComponent::onSizeChanged()
{
	buildVertices();
}

const Vector2f& NinePatchComponent::getCornerSize() const
{
	return mCornerSize;
}

void NinePatchComponent::setCornerSize(int sizeX, int sizeY)
{
	mCornerSize = Vector2f(sizeX, sizeY);
	buildVertices();
}

void NinePatchComponent::fitTo(Vector2f size, Vector3f position, Vector2f padding)
{
	size += padding;
	position[0] -= padding.x() / 2;
	position[1] -= padding.y() / 2;

	setSize(size + mCornerSize * 2);
	setPosition(position.x() + Math::lerp(-mCornerSize.x(), mCornerSize.x(), mOrigin.x()),
				position.y() + Math::lerp(-mCornerSize.y(), mCornerSize.y(), mOrigin.y()));
}

void NinePatchComponent::setImagePath(const std::string& path)
{
	mPath = path;
	buildVertices();
}

void NinePatchComponent::setEdgeColor(unsigned int edgeColor)
{
	mEdgeColor = edgeColor;
	updateColors();
}

void NinePatchComponent::setCenterColor(unsigned int centerColor)
{
	mCenterColor = centerColor;
	updateColors();
}

void NinePatchComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "ninepatch");
	if(!elem)
		return;

	if(properties & PATH && elem->has("path"))
		setImagePath(elem->get<std::string>("path"));
}
