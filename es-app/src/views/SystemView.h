#pragma once

#include "GuiComponent.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "components/ScrollableContainer.h"
#include "components/IList.h"
#include "resources/TextureResource.h"

class SystemData;
class AnimatedImageComponent;

enum CarouselType : unsigned int
{
	HORIZONTAL = 0,
	VERTICAL = 1
};

struct SystemViewData
{
	std::shared_ptr<GuiComponent> logo;
	std::shared_ptr<GuiComponent> logoSelected;
	std::shared_ptr<ThemeExtras> backgroundExtras;
};

struct SystemViewCarousel
{
	CarouselType type;
	Eigen::Vector2f pos;
	Eigen::Vector2f size;
	float logoScale;
	Eigen::Vector2f logoSpacing;
	unsigned int color;
	int maxLogoCount; // number of logos shown on the carousel
	Eigen::Vector2f logoSize;
};

class SystemView : public IList<SystemViewData, SystemData*>
{
public:
	SystemView(Window* window);

	void goToSystem(SystemData* system, bool animate);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	std::vector<HelpPrompt> getHelpPrompts() override;
	virtual HelpStyle getHelpStyle() override;

protected:
	void onCursorChanged(const CursorState& state) override;

private:
	void populate();
	void getViewElements(const std::shared_ptr<ThemeData>& theme);
	void getDefaultElements(void);
	void getCarouselFromTheme(const ThemeData::ThemeElement* elem);

	void renderCarousel(const Eigen::Affine3f& parentTrans);
	void renderExtras(const Eigen::Affine3f& parentTrans);
	void renderInfoBar(const Eigen::Affine3f& trans);

	SystemViewCarousel mCarousel;
	TextComponent mSystemInfo;

	// unit is list index
	float mCamOffset;
	float mExtrasCamOffset;
	float mExtrasFadeOpacity;

	bool mViewNeedsReload;
};
