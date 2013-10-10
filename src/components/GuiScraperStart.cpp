#include "GuiScraperStart.h"

GuiScraperStart::GuiScraperStart(Window* window) : GuiComponent(window),
	mBox(window, ":/frame.png"),
	mList(window, Eigen::Vector2i(2, 4)),
	mFilterLabel(mWindow),
	mSystemsLabel(mWindow),
	mManualLabel(mWindow),
	mFiltersOpt(mWindow),
	mSystemsOpt(mWindow),
	mManualSwitch(mWindow)
{
	mFilterLabel.setText("Filter: ");
	mSystemsLabel.setText("Systems: ");
	mManualLabel.setText("Manual mode: ");

	addChild(&mBox);
	addChild(&mList);

	using namespace Eigen;
	mList.setEntry(Vector2i(0, 0), Vector2i(1, 1), &mFilterLabel, false, ComponentListComponent::AlignRight);
	mList.setEntry(Vector2i(1, 0), Vector2i(1, 1), &mFiltersOpt, true, ComponentListComponent::AlignCenter);

	mList.setEntry(Vector2i(0, 1), Vector2i(1, 1), &mSystemsLabel, false, ComponentListComponent::AlignRight);
	mList.setEntry(Vector2i(1, 1), Vector2i(1, 1), &mSystemsOpt, true, ComponentListComponent::AlignCenter);

	mList.setEntry(Vector2i(0, 2), Vector2i(1, 1), &mManualLabel, false, ComponentListComponent::AlignRight);
	mList.setEntry(Vector2i(1, 2), Vector2i(1, 1), &mManualSwitch, true, ComponentListComponent::AlignCenter);

	mBox.fitTo(mList.getSize());
}

