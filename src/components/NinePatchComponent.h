#pragma once

#include "../GuiComponent.h"
#include "../resources/TextureResource.h"

class NinePatchComponent : public GuiComponent
{
public:
	NinePatchComponent(Window* window, const std::string& path = "", unsigned int edgeColor = 0xFFFFFFFF, unsigned int centerColor = 0xFFFFFFFF);

	void render(const Eigen::Affine3f& parentTrans) override;

	void onSizeChanged() override;

	void fitTo(Eigen::Vector2f size, Eigen::Vector3f position = Eigen::Vector3f::Zero(), Eigen::Vector2f padding = Eigen::Vector2f::Zero());

	void setImagePath(const std::string& path);
	void setEdgeColor(unsigned int edgeColor);
	void setCenterColor(unsigned int centerColor);

private:
	Eigen::Vector2f getCornerSize() const;

	void buildVertices();
	void updateColors();

	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	};

	Vertex* mVertices;
	GLubyte* mColors;

	std::string mPath;
	unsigned int mEdgeColor;
	unsigned int mCenterColor;
	std::shared_ptr<TextureResource> mTexture;
};
