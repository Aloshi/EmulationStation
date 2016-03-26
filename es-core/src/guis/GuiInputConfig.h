#pragma once

#include "GuiComponent.h"
#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include "components/ComponentList.h"
#include "components/BusyComponent.h"

class TextComponent;

class GuiInputConfig : public GuiComponent
{
public:
	GuiInputConfig(Window* window, InputConfig* target, bool reconfigureAll, const std::function<void()>& okCallback);

	void update(int deltaTime) override;

	void onSizeChanged() override;

private:
	void error(const std::shared_ptr<TextComponent>& text, const std::string& msg); // set text to "msg" + not greyed out

	void setPress(const std::shared_ptr<TextComponent>& text); // set text to "PRESS ANYTHING" + not greyed out
	void setNotDefined(const std::shared_ptr<TextComponent>& text); // set text to -NOT DEFINED- + greyed out
	void setAssignedTo(const std::shared_ptr<TextComponent>& text, Input input); // set text to "BUTTON 2"/"AXIS 2+", etc.

	bool assign(Input input, int inputId);
	void clearAssignment(int inputId);

	void rowDone();

	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextComponent> mSubtitle1;
	std::shared_ptr<TextComponent> mSubtitle2;
	std::shared_ptr<ComponentList> mList;
	std::vector< std::shared_ptr<TextComponent> > mMappings;
	std::shared_ptr<ComponentGrid> mButtonGrid;

	InputConfig* mTargetConfig;
	bool mConfiguringRow; // next input captured by mList will be interpretted as a remap
	bool mConfiguringAll; // move the cursor down after configuring a row and start configuring the next row until we reach the bottom

	bool mHoldingInput;
	Input mHeldInput;
	int mHeldTime;
	int mHeldInputId;

	BusyComponent mBusyAnim;	
};
