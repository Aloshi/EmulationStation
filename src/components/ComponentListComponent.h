#pragma once

#include "../GuiComponent.h"

class ComponentListComponent : public GuiComponent
{
public:
	ComponentListComponent(Window* window, Eigen::Vector2i gridDimensions);

	enum UpdateBehavior
	{
		UpdateAlways, UpdateFocused
	};

	enum AlignmentType
	{
		AlignLeft, AlignRight, AlignCenter
	};

	//DO NOT USE NEGATIVE NUMBERS FOR POSITION OR SIZE.
	void setEntry(Eigen::Vector2i pos, Eigen::Vector2i size, GuiComponent* component, bool canFocus, AlignmentType align, Eigen::Matrix<bool, 1, 2> autoFit, UpdateBehavior updateType = UpdateAlways);

	void onPositionChanged() override;

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void setColumnWidth(int col, unsigned int size);
	void setRowHeight(int row, unsigned int size);

	void resetCursor();
	bool cursorValid();

	GuiComponent* getSelectedComponent();

private:
	class ComponentEntry
	{
	public:
		Eigen::Vector2i pos;
		Eigen::Vector2i dim;
		GuiComponent* component;
		UpdateBehavior updateType;
		AlignmentType alignment;
		bool canFocus;

		ComponentEntry() : component(NULL), updateType(UpdateAlways), canFocus(true), alignment(AlignCenter) {};
		ComponentEntry(Eigen::Vector2i p, Eigen::Vector2i d, GuiComponent* comp, UpdateBehavior update, bool focus, AlignmentType align) : pos(p), dim(d), component(comp), updateType(update), canFocus(focus), alignment(align) {};

		operator bool() const
		{
			return component != NULL;
		}
	};

	//Offset we render components by (for scrolling).
	Eigen::Vector2f mComponentOffset;

	Eigen::Vector2i mGridSize;
	ComponentEntry** mGrid;
	std::vector<ComponentEntry> mEntries;
	void makeCells(Eigen::Vector2i size);
	void setCell(unsigned int x, unsigned int y, ComponentEntry* entry);
	ComponentEntry* getCell(unsigned int x, unsigned int y);

	Eigen::Vector2i mSelectedCellIndex;

	unsigned int getColumnWidth(int col);
	unsigned int getRowHeight(int row);

	unsigned int* mColumnWidths;
	unsigned int* mRowHeights;

	Eigen::Vector3f getCellOffset(Eigen::Vector2i gridPos);
	void updateSize();

	void moveCursor(Eigen::Vector2i dir);
	Eigen::Vector2i mCursor;

	void updateComponentOffsets();
};

//ability to define a list of components in terms of a grid
//these comments are kinda old

//input
//pass to selected component
//  if returns true, stop
//   else, process:
//     if input == up/down
//       scroll to prev/next selectable component in grid Y
//     if input == left/right
//       scroll to prev/next selectable component in grid X
//     if input == accept
//       call registered function?

//entry struct/class
//  GuiComponent* component - component to work with
//  bool canFocus - can we pass input to this? (necessary for labels to not be selectable)
//  Function* selectFunc?
//  UpdateBehavior update - how to handle updates (all the time or only when focused)

//update
//animate component offset to display selected component within the bounds
//pass update to all entries with appropriate update behavior

//render
//clip rect to our size
//render a "selected" effect behind component with focus somehow
//  an edge filter would be cool, but we can't really do that without shader support
//  a transparent rect will work for now, but it's kind of ugly...maybe a GuiBox
//glTranslatef by our render offset
//  doesn't handle getGlobalOffset for our components...would need parenting for that

//methods
//List::setEntry(Vector2i gridPos, GuiComponent* component, bool canFocus, AlignmentType align, 
//		Function* selectFunc = NULL, UpdateBehavior updateType = UpdateAlways);

//example of setting up the SettingsMenu list:
//ComponentListComponent list;
//int row = 0;
//TextComponent* label = new TextComponent(Vector2i(0, 0), "Debug:", font, lblColor, etc);
//
//list.setEntry(Vector2i(-1, row), label, false, AlignRight);
//list.setEntry(Vector2i(0, row++), &mDebugSwitch, true, AlignLeft);
//...
//list.setEntry(Rect(-1, row, 2, 1), &mSaveButton, true, AlignCenter);

//example of setting up GameGrid list:
//ComponentListComponent list;
//for(int y = 0; y < yMax; y++)
//	for(int x = 0; x < xMax; x++)
//		list.setEntry(Vector2i(x, y), getGameImage(x, y), true, AlignCenter, &this->onSelectGame);
