#pragma once

#include "../GuiComponent.h"
#include "../resources/TextureResource.h"

class NinePatchComponent : public GuiComponent
{
public:
	NinePatchComponent(Window* window, const std::string& path, unsigned int edgeColor = 0xFFFFFFFF, unsigned int centerColor = 0xFFFFFFFF);

	void render(const Eigen::Affine3f& parentTrans) override;

	void onSizeChanged() override;

	void fitTo(Eigen::Vector2f size);

private:
	Eigen::Vector2f getCornerSize() const;

	void buildVertices();

	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	};

	Vertex* mVertices;
	GLubyte* mColors;

	unsigned int mEdgeColor;
	unsigned int mCenterColor;
	std::shared_ptr<TextureResource> mTexture;
};
