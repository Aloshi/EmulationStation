#pragma once

#include "../GuiComponent.h"
#include "../components/ImageComponent.h"
#include "../components/TextComponent.h"
#include "../components/ScrollableContainer.h"
#include "../components/IList.h"
#include "../resources/TextureResource.h"

class SystemData;

struct SystemViewData
{
	std::shared_ptr<TextComponent> title;
	std::shared_ptr<GuiComponent> logo;
	std::shared_ptr<ImageComponent> background;
};

class SystemView : public IList<SystemViewData, SystemData*>
{
public:
	SystemView(Window* window);

	void goToSystem(SystemData* system);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	std::vector<HelpPrompt> getHelpPrompts() override;
	
protected:
	void onCursorChanged(const CursorState& state) override;

private:
	inline Eigen::Vector2f logoSize() const { return Eigen::Vector2f(mSize.x() * 0.3f, mSize.y() * 0.25f); }

	void populate();

	// unit is list index
	float mCamOffset;
};
