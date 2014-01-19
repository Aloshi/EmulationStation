#pragma once

#include "../GuiComponent.h"
#include "../resources/TextureResource.h"

class RatingComponent : public GuiComponent
{
public:
	RatingComponent(Window* window);

	std::string getValue() const override;
	void setValue(const std::string& value) override;

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans);

	void onSizeChanged() override;

	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

private:
	void updateVertices();

	float mValue;

	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	} mVertices[12];

	std::shared_ptr<TextureResource> mFilledTexture;
	std::shared_ptr<TextureResource> mUnfilledTexture;
};

