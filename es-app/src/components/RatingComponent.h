#pragma once

#include "GuiComponent.h"
#include "resources/TextureResource.h"

#define NUM_RATING_STARS 5

// Used to visually display/edit some sort of "score" - e.g. 5/10, 3/5, etc.
// setSize(x, y) works a little differently than you might expect:
//   * (0, y != 0) - x will be automatically calculated (5*y).
//   * (x != 0, 0) - y will be automatically calculated (x/5).
//   * (x != 0, y != 0) - you better be sure x = y*5
class RatingComponent : public GuiComponent
{
public:
	RatingComponent(Window* window);

	std::string getValue() const override;
	void setValue(const std::string& value) override; // Should be a normalized float (in the range [0..1]) - if it's not, it will be clamped.

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans);

	void onSizeChanged() override;

	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

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

