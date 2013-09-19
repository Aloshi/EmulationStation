#pragma once

#include "../GuiComponent.h"

class ComponentListComponent : public GuiComponent
{
public:
	ComponentListComponent(Window* window, Eigen::Vector2i gridDimensions);
	virtual ~ComponentListComponent();

	enum UpdateBehavior
	{
		UpdateAlways, UpdateFocused
	};

	enum AlignmentType
	{
		AlignLeft, AlignRight, AlignCenter
	};

	//DO NOT USE NEGATIVE NUMBERS FOR POSITION OR SIZE.
	void setEntry(Eigen::Vector2i pos, Eigen::Vector2i size, GuiComponent* component, bool canFocus, AlignmentType align, 
		Eigen::Matrix<bool, 1, 2> autoFit = Eigen::Matrix<bool, 1, 2>(true, true), UpdateBehavior updateType = UpdateAlways);

	void removeEntriesIn(Eigen::Vector2i pos, Eigen::Vector2i size);

	void onPositionChanged() override;

	void textInput(const char* text) override;
	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void forceColumnWidth(int col, unsigned int size);
	void forceRowHeight(int row, unsigned int size);

	void updateComponent(GuiComponent* cmp);

	void resetCursor();
	bool cursorValid();

	GuiComponent* getSelectedComponent();

	void moveCursor(Eigen::Vector2i dir);

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

	//Offset we render components by (for scrolling). [unimplemented]
	Eigen::Vector2f mComponentOffset;

	Eigen::Vector2i mGridSize;
	ComponentEntry** mGrid;
	std::vector<ComponentEntry*> mEntries;
	void makeCells(Eigen::Vector2i size);
	void setCell(unsigned int x, unsigned int y, ComponentEntry* entry);
	ComponentEntry* getCell(unsigned int x, unsigned int y);

	unsigned int getColumnWidth(int col);
	unsigned int getRowHeight(int row);

	unsigned int* mColumnWidths;
	unsigned int* mRowHeights;
	bool* mColumnWidthForced;
	bool* mRowHeightForced;

	Eigen::Vector3f getCellOffset(Eigen::Vector2i gridPos);
	void updateSize();

	void onCursorMoved(Eigen::Vector2i from, Eigen::Vector2i to);
	Eigen::Vector2i mCursor;

	void updateComponentOffsets();
	void updateCellSize(ComponentEntry* e, bool updWidth = true, bool updHeight = true);
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

//entry struct/class
//  GuiComponent* component - component to work with
//  bool canFocus - can we pass input to this? (necessary for labels to not be selectable)
//  UpdateBehavior update - how to handle updates (all the time or only when focused)

//update
//pass update to all entries with appropriate update behavior

//render
//clip rect to our size
//render a "selected" effect behind component with focus somehow
