#pragma once

#include "../GuiComponent.h"

class ComponentListComponent : public GuiComponent
{
public:
	ComponentListComponent(Window* window, Vector2u gridDimensions);

	enum UpdateBehavior
	{
		UpdateAlways, UpdateFocused
	};

	enum AlignmentType
	{
		AlignLeft, AlignRight, AlignCenter
	};

	void setEntry(Vector2u pos, Vector2u size, GuiComponent* component, bool canFocus, AlignmentType align, Vector2<bool> autoFit, UpdateBehavior updateType = UpdateAlways);

	void onOffsetChanged() override;

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void onRender() override;

	void setColumnWidth(int col, unsigned int size);
	void setRowHeight(int row, unsigned int size);

	void resetCursor();
	bool cursorValid();

	GuiComponent* getSelectedComponent();

private:
	class ComponentEntry
	{
	public:
		Rect box;
		GuiComponent* component;
		UpdateBehavior updateType;
		AlignmentType alignment;
		bool canFocus;

		ComponentEntry() : component(NULL), updateType(UpdateAlways), canFocus(true), alignment(AlignCenter) {};
		ComponentEntry(Rect b, GuiComponent* comp, UpdateBehavior update, bool focus, AlignmentType align) : box(b), component(comp), updateType(update), canFocus(focus), alignment(align) {};

		operator bool() const
		{
			return component != NULL;
		}
	};

	//Offset we render components by (for scrolling).
	Vector2i mComponentOffset;

	Vector2u mGridSize;
	ComponentEntry** mGrid;
	std::vector<ComponentEntry> mEntries;
	void makeCells(Vector2u size);
	void setCell(unsigned int x, unsigned int y, ComponentEntry* entry);
	ComponentEntry* getCell(unsigned int x, unsigned int y);

	Vector2u mSelectedCellIndex;

	unsigned int getColumnWidth(int col);
	unsigned int getRowHeight(int row);

	unsigned int* mColumnWidths;
	unsigned int* mRowHeights;

	Vector2i getCellOffset(Vector2u gridPos);
	void updateSize();

	void moveCursor(Vector2i dir);
	Vector2i mCursor;

	void updateComponentOffsets();
};

//ability to define a list of components in terms of a grid

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
